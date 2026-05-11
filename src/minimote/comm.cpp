#include "minimote/comm.h"

#if MINIMOTE

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_mac.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "minimote/mqtt.h"
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
#ifndef MQTT_PASSWORD
    #define MQTT_PASSWORD ""
#endif

static char endpoint[32];
static char user[64];
static char topicInit[64];
static char topicData[64];
static char topicControl[64];
static char topicAck[64];
static bool controlSubscribed = false;

static bool minimote_handle_control_message(const String &topic, const String &payload) {
    if (topic != String(topicControl)) {
        return false;
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
    if (strcmp(cmd, "reboot") == 0) {
        // We can't quite reboot yet since we still need to acknowledge the command, so do nothing for now
    } else if (strcmp(cmd, "ota") == 0) {
        // TODO: implement OTA update (specify url to download at)
    } else {
        Serial.println("unknown control command");
        return false;
    }
    // Acknowledge we received the command
    char ackPayload[192];
    snprintf(ackPayload, sizeof(ackPayload), "{\"ok\":true,\"cmd\":\"%s\"}", cmd);
    mqtt_publish(topicAck, ackPayload, 1, false);
    // Special case for reboot command as specified above
    if (strcmp(cmd, "reboot") == 0) {
        delay(200); // Ensure ack is sent
        ESP.restart();
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

    if (!mqtt_init(endpoint, user, MQTT_PASSWORD)) {
        Serial.println("mqtt init failed");
        return false;
    }
    controlSubscribed = false;
    return true;
}

bool minimote_comm_sync_time() {
    if (time(nullptr) > 1700000000) {
        // Time is recent, so has been synced
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
    return true;
}

void minimote_comm_control_window(uint32_t window) {
    if (!controlSubscribed) {
        return;
    }
    unsigned long start = millis();
    while (millis() - start < window) {
        // While the control window is open, read messages and check for control commands to execute
        String topic, payload;
        if (!mqtt_read_message(topic, payload, 150)) {
            continue;
        }
        Serial.printf("control msg topic=%s payload=%s\n", topic.c_str(), payload.c_str());
        if (minimote_handle_control_message(topic, payload)) {
            return;
        }
    }
}

bool minimote_comm_subscribe_control() {
    if (controlSubscribed) {
        return true;
    }
    if (!mqtt_is_connected()) {
        Serial.println("mqtt unavailable for control subscribe");
        return false;
    }
    if (!mqtt_subscribe(topicControl, 1)) {
        Serial.println("failed to subscribe control topic");
        return false;
    }
    controlSubscribed = true;
    return true;
}

bool minimote_comm_unsubscribe_control() {
    if (!controlSubscribed) {
        return true;
    }
    if (!mqtt_is_connected()) {
        controlSubscribed = false;
        return true;
    }
    if (!mqtt_unsubscribe(topicControl)) {
        Serial.println("failed to unsubscribe control topic");
        return false;
    }
    controlSubscribed = false;
    return true;
}

bool minimote_comm_publish_sample() {
    if (!mqtt_is_connected()) {
        Serial.println("mqtt unavailable for sample publish");
        return false;
    }

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
    char payload[1024];
    snprintf(payload, sizeof(payload),
        "{\"unix_time\":%ld,\"millis\":%lu,\"battery_v\":%.3f,\"battery_pct\":%d,\"vbus_v\":%.3f,\"charging\":%s,\"water_temp\":%.2f,\"turbidity\":%.2f,\"tds\":%.2f,\"air_velocity\":%.2f,\"ozone\":%.4f,\"air_temp\":%.2f,\"humidity\":%.2f,\"uv\":%.2f,\"lum\":%.2f,\"baro\":%.2f,\"pm1_0\":%.2f,\"pm2_5\":%.2f,\"pm10\":%.2f}",
        time(nullptr), millis(),
        battV, battPct, vbusV, charging ? "true" : "false",
        temperature, turbidity, tds,
        velo, ozone, env.tempC, env.hum, env.uv, env.lum, env.baro,
        pm.pm1_0, pm.pm2_5, pm.pm10
    );

    bool published = mqtt_publish(topicData, payload);
    Serial.printf("sample publish %s\n", published ? "ok" : "failed");
    return published;
}

bool minimote_comm_publish_init(const char *init) {
    if (!mqtt_is_connected()) {
        Serial.println("mqtt unavailable for init publish");
        return false;
    }
    bool published = mqtt_publish(topicInit, init);
    Serial.printf("init publish %s\n", published ? "ok" : "failed");
    return published;
}

#endif // MINIMOTE
