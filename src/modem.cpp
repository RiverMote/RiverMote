// https://edworks.co.kr/wp-content/uploads/2022/04/SIM7070_SIM7080_SIM7090-Series_AT-Command-Manual_V1.05.pdf

#include <HardwareSerial.h>
#include <string.h>
#include <time.h>
#include <TinyGsmClient.h>

#include "pins.h"

#include "modem.h"

#if LOG_MODEM
    #include <StreamDebugger.h>
    StreamDebugger debugger(Serial1, Serial);
    TinyGsm modem(debugger);
#else
    TinyGsm modem(Serial1);
#endif

// 1nce GPRS settings
#define GPRS_APN "iot.1nce.net"
#define GPRS_USER ""
#define GPRS_PASS ""

static bool gpsEnabled = false, gpsFixed = false;

/* -- General modem functions -- */

bool modem_init(unsigned long baud, uint max_retries) {
    // Initialize modem serial communication
    Serial1.begin(baud, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
    pinMode(PIN_MODEM_PWR, OUTPUT);
    pinMode(PIN_MODEM_DTR, OUTPUT);
    digitalWrite(PIN_MODEM_DTR, LOW);

    // Pulse modem power to reset it
    Serial.println("resetting modem");
    digitalWrite(PIN_MODEM_PWR, LOW);
    delay(100);
    digitalWrite(PIN_MODEM_PWR, HIGH);
    delay(1000);
    digitalWrite(PIN_MODEM_PWR, LOW);

    // Wait for the modem to start
    Serial.print("waiting for modem to start");
    uint retry = 0;
    while (!modem.testAT(1000) && retry < max_retries) {
        Serial.print(".");
        retry++;
    }
    Serial.println();
    if (retry > max_retries) {
        return false;
    }

    // Enable sleep mode via DTR pin
    return modem.sleepEnable(true);
}

bool modem_send(const char *cmd, uint32_t timeout) {
    modem.sendAT(cmd);
    return modem.waitResponse(timeout) == 1;
}

void modem_set_sleep(bool enable) {
    digitalWrite(PIN_MODEM_DTR, enable);
    delay(10);
}

void modem_deinit() {
    // Disable GPS and cellular if enabled
    if (gpsEnabled) {
        gpsEnabled = !modem.disableGPS();
    }
    if (modem.isGprsConnected()) {
        modem.gprsDisconnect();
    }
    // Gracefully shut down down, then cut power
    if (!modem.poweroff()) {
        Serial.println("WARNING: failed to send modem shutdown, cutting power anyway");
    }
    delay(200);
    digitalWrite(PIN_MODEM_PWR, HIGH);
    Serial1.end();
}

/* -- GPS functions -- */

bool modem_gps_enable() {
    if (modem.isGprsConnected()) {
        Serial.println("cell enabled, can't enable GPS");
        return false;
    }
    gpsEnabled = modem.enableGPS();
    return gpsEnabled;
}

bool modem_gps_is_enabled() {
    return gpsEnabled;
}

ModemGPSData modem_gps_read() {
    // Get raw GPS data from module
    ModemGPSData data;
    String raw = modem.getGPSraw();
    // Parse returned data; see page 202 of the above datasheet for field info
    int parsed = sscanf(raw.c_str(), "%*[^,],%d,%*[^,],%lf,%lf,%*[^,],%f,%f", &gpsFixed, &data.lat, &data.lng, &data.speed, &data.track);
    if (parsed < 5) {
        data.lat = data.lng = data.speed = data.track = 0.f;
    }
    return data;
}

String modem_gps_read_time() {
    if (gpsFixed) {
        String raw = modem.getGPSraw();
        return raw.substring(4, 18); // Position 3 of raw field
    } else {
        return String("00000000000000");
    }
}

bool modem_gps_fixed() {
    (void)modem_gps_read();
    return gpsFixed;
}

/* -- Cellular functions -- */

bool modem_cell_enable() {
    if (gpsEnabled) {
        Serial.println("GPS enabled, can't enable cell");
        return false;
    }
    if (modem.getSimStatus() != SIM_READY) {
        Serial.println("SIM not ready");
        return false;
    }

    modem.setNetworkMode(38); // LTE only
    modem.setPreferredMode(1); // Prefer CAT-M

    // Register to network
    Serial.print("registering to network (may take a moment)");
    SIM70xxRegStatus status;
    do {
        status = modem.getRegistrationStatus();
        if (status == REG_SEARCHING) {
            Serial.print(".");
        } else {
            Serial.print("!");
        }
        delay(1000);
    } while (status != REG_OK_HOME && status != REG_OK_ROAMING);
    Serial.print("\nregistration status: ");
    switch (status) {
        case REG_UNREGISTERED: Serial.println("not registered"); break;
        case REG_SEARCHING: Serial.println("searching"); break;
        case REG_DENIED: Serial.println("denied"); break;
        case REG_UNKNOWN: Serial.println("unknown"); break;
        case REG_OK_HOME: Serial.println("registered (home)"); break;
        case REG_OK_ROAMING: Serial.println("registered (roaming)"); break;
        default: Serial.println("invalid"); break;
    }
    if (status != REG_OK_HOME && status != REG_OK_ROAMING) {
        Serial.println("registration failed");
        return false;
    }

    // Activate GPRS
    Serial.print("activating gprs...");
    if (!modem.gprsConnect(GPRS_APN, GPRS_USER, GPRS_PASS)) {
        Serial.println("GPRS connection failed");
        return false;
    }
    Serial.printf("activated! IP: %s, signal: %.2f\n", modem.getLocalIP().c_str(), modem.getSignalQuality() / 31.f);
    
    return true;
}

bool modem_cell_is_connected() {
    return modem.isGprsConnected();
}

time_t modem_cell_read_time() {
    modem_send("+CLTS=1"); // Enable network time synchronization URC
    // Get individual time components
    int yyyy = 0, mo = 0, dd = 0, hh = 0, mm = 0, ss = 0;
    float tz = 0.f; // Timezone offset in hours, either positive or negative from UTC
    if (!modem.getNetworkTime(&yyyy, &mo, &dd, &hh, &mm, &ss, &tz)) {
        return 0;
    }
    // Fill in a tm struct and convert to epoch time
    tm t = {};
    t.tm_year = yyyy - 1900;
    t.tm_mon = mo - 1;
    t.tm_mday = dd;
    t.tm_hour = hh;
    t.tm_min = mm;
    t.tm_sec = ss;
    t.tm_isdst = 0;
    time_t ts = mktime(&t);
    if (ts <= 0) {
        return 0;
    }
    // Apply timezone correction (the modem returns local time but we want epoch in UTC)
    ts -= (time_t)(tz * 3600.f); // 3600 seconds in an hour
    return ts;
}

/* -- SM (MQTT) functions -- */

bool modem_mqtt_is_connected() {
    modem.sendAT("+SMSTATE?");
    if (modem.waitResponse("+SMSTATE: ")) {
        String res = modem.stream.readStringUntil('\r');
        return res.toInt();
    }
    return false;
}

bool modem_set_mqtt_ssl(bool enable) {
    char cmd[24];
    snprintf(cmd, sizeof(cmd), "+SMSSL=%d,\"\",\"\"", enable ? 1 : 0);
    return modem_send(cmd);
}

bool modem_send_smpub(const char *cmd, const char *payload, size_t payload_len) {
    modem.sendAT(cmd);
    if (modem.waitResponse(">") == 1) {
        modem.stream.write(payload, payload_len);
        if (modem.waitResponse(3000)) {
            return true;
        }
    }
    return false;
}

bool modem_send_smsub(const char *topic, uint8_t qos) {
    char cmd[192];
    snprintf(cmd, sizeof(cmd), "+SMSUB=\"%s\",%u", topic, qos > 1 ? 1 : qos);
    return modem_send(cmd);
}

bool modem_read_line(String &line, uint32_t timeout) {
    line = "";
    uint32_t start = millis();
    while ((millis() - start) < timeout || timeout == 0) {
        while (modem.stream.available()) {
            char c = modem.stream.read();
            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                line.trim();
                if (line.length() > 0) {
                    return true;
                }
                continue;
            }
            line += c;
        }
        if (timeout == 0) {
            break;
        }
        delay(5);
    }
    line.trim();
    return line.length() > 0;
}
