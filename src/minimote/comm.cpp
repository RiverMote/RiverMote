#include "minimote/comm.h"

#if MINIMOTE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_mac.h>
#include <PubSubClient.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "minimote/ota.h"
#include "sensors/air_velocity.h"
#include "sensors/env.h"
#include "sensors/ozone.h"
#include "sensors/pm_sensor.h"
#include "sensors/tds.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"
#include "modem.h"
#include "pmu.h"
#if __has_include("secrets.h")
    #include "secrets.h"
#endif

// The below configuration options can be overridden by defining them in secrets.h or elsewhere.
#ifndef MQTT_BROKER
    #define MQTT_BROKER ""
#endif
#ifndef MQTT_PORT
    #define MQTT_PORT 1883
#endif
#ifndef MQTT_PASSWORD
    #define MQTT_PASSWORD ""
#endif
#define MAX_MQTT_PAYLOAD_SIZE 512
#define TIME_SYNC_INTERVAL_MS 1500000 // How often to sync time, in ms (25 mins)

static PubSubClient mqtt(MQTT_BROKER, MQTT_PORT, modemClient);
static char endpoint[32], user[64];
static char topicInit[64], topicData[64], topicControl[64], topicAck[64];
static unsigned long lastTimeSync = 0;

static bool ensure_mqtt() {
    if (mqtt.connected()) {
        return true;
    }
    Serial.println("mqtt disconnected, attempting reconnect");
    if (!mqtt.connect(endpoint, user, MQTT_PASSWORD)) {
        Serial.println("mqtt connect failed!");
        return false;
    }
    // Now that we are reconnected, we need to resubscribe to control
    if (!mqtt.subscribe(topicControl, 1)) {
        Serial.println("failed to subscribe to control topic!");
        return false;
    }
    return true;
}

static bool handle_control(char *topic, byte *payload, unsigned int len) {
    if (strcasecmp(topic, topicControl) != 0) {
        return false; // Not a control message
    }
    // Parse payload and extract command
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err || !doc["cmd"].is<const char*>()) {
        Serial.printf("invalid control message json: %s\n", err.c_str());
        return false;
    }
    const char *cmd = doc["cmd"];
    Serial.printf("received control command: %s\n", cmd);
    
    // Acknowledge we received the command
    char ackPayload[192];
    snprintf(ackPayload, sizeof(ackPayload), "{\"ok\":true,\"cmd\":\"%s\"}", cmd);
    mqtt.publish(topicAck, ackPayload, true);
    delay(500); // Ensure ack is sent
    
    // Execute the command
    if (strcasecmp(cmd, "reboot") == 0) {
        ESP.restart();
    } else if (strcasecmp(cmd, "ota") == 0) {
        minimote_comm_deinit(); // Deinit so we can connect to HTTP
        if (!ota_do_update(doc["server"].as<const char*>(), doc["path"].as<const char*>())) {
            Serial.println("OTA update failed");
            return false;
        }
        delay(1000);
        ESP.restart();
    } else {
        Serial.println("unknown control command");
        return false;
    }
    return true;
}

bool minimote_comm_init() {
    // Create a device endpoint based off the device's unique MAC address, which will be used to generate a unique
    // client ID, username, and topics for this device
    uint8_t mac[6] = {0};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(endpoint, sizeof(endpoint), "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snprintf(user, sizeof(user), "minimote-%s", endpoint);
    snprintf(topicInit, sizeof(topicInit), "minimote/%s/init", endpoint);
    snprintf(topicData, sizeof(topicData), "minimote/%s/data", endpoint);
    snprintf(topicControl, sizeof(topicControl), "minimote/%s/control", endpoint);
    snprintf(topicAck, sizeof(topicAck), "minimote/%s/ack", endpoint);
    Serial.printf("device endpoint=%s user=%s topicInit=%s topicData=%s topicControl=%s topicAck=%s\n", endpoint, user, topicInit, topicData, topicControl, topicAck);
    // Connect to MQTT and subscribe to control topic
    mqtt.setKeepAlive(60);
    mqtt.setCallback(handle_control);
    mqtt.setBufferSize(MAX_MQTT_PAYLOAD_SIZE);
    return ensure_mqtt();
}

void minimote_comm_deinit() {
    mqtt.disconnect();
}

bool minimote_comm_sync_time(bool force) {
    if (!force && millis() - lastTimeSync < TIME_SYNC_INTERVAL_MS) {
        // Time synced recently
        return true;
    }

    // Get NTP from modem
    time_t epoch = modem_cell_read_time();
    if (epoch <= 0) {
        Serial.println("failed to sync time");
        return false;
    } else if (epoch < 1700000000) {
        Serial.printf("time synced, but seems wrong? (%ld)\n", epoch);
        return false;
    }

    // Syncronize system clock with NTP time
    timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
    Serial.printf("time synced, epoch=%ld\n", epoch);
    lastTimeSync = millis();
    return true;
}

void minimote_comm_control_window(uint32_t window) {
    if (!ensure_mqtt()) {
        return;
    }
    unsigned long start = millis();
    while (millis() - start < window) {
        mqtt.loop();
    }
}

bool minimote_comm_subscribe_control() {
    if (!mqtt.subscribe(topicControl, 1)) {
        Serial.println("failed to subscribe control topic");
        return false;
    }
    return true;
}

bool minimote_comm_unsubscribe_control() {
    if (!mqtt.unsubscribe(topicControl)) {
        Serial.println("failed to unsubscribe control topic");
        return false;
    }
    return true;
}

bool minimote_comm_publish_sample() {
    if (!ensure_mqtt()) {
        Serial.println("mqtt unavailable for sample publish");
        return false;
    }
    // Sync time to ensure our timestamp stays accurate (esp clock can drift)
    // Do not force sync; we only need to sync periodically and the function can skip if necessary
    minimote_comm_sync_time(false);

    // Harvest all sensor data
    float battV = pmu_get_battery_voltage();
    int battPct = pmu_get_battery_percent();
    float vbusV = pmu_get_vbus_voltage();
    bool charging = pmu_is_charging();
    float temperature = temp_read();
    float turbidity = get_turbidity();
    float tds = get_tds();
    float velo = velo_read();
    double ozone = ozone_read();
    EnvData env = env_read();
    PMData pm = pm_read();
    // Construct JSON payload and publish to MQTT topic
    char payload[512];
    snprintf(payload, sizeof(payload),
        "{\"unix_time\":%ld,\"millis\":%lu,\"battery_v\":%.3f,\"battery_pct\":%d,\"water_temp\":%.2f,\"turbidity\":%.2f,\"tds\":%.2f,\"air_velocity\":%.2f,\"ozone\":%.4f,\"air_temp\":%.2f,\"humidity\":%.2f,\"uv\":%.2f,\"lum\":%.2f,\"baro\":%.2f,\"pm1_0\":%.2f,\"pm2_5\":%.2f,\"pm10\":%.2f}",
        time(nullptr), millis(),
        battV, battPct,
        temperature, turbidity, tds,
        velo, ozone, env.tempC, env.hum, env.uv, env.lum, env.baro,
        pm.pm1_0, pm.pm2_5, pm.pm10
    );

    bool published = mqtt.publish(topicData, payload);
    Serial.printf("sample publish %s\n", published ? "ok" : "failed");
    return published;
}

bool minimote_comm_publish_init(const char *init) {
    if (!ensure_mqtt()) {
        Serial.println("mqtt unavailable for init publish");
        return false;
    }
    bool published = mqtt.publish(topicInit, init);
    Serial.printf("init publish %s\n", published ? "ok" : "failed");
    return published;
}

#endif // MINIMOTE
