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
#include "minimote/power.h"
#include "sensors/air_velocity.h"
#include "sensors/chamber.h"
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

static PubSubClient mqtt(MQTT_BROKER, MQTT_PORT, modemClient);
static char endpoint[32], user[64];
static char topicInit[64], topicData[64], topicControl[64], topicAck[64];

static bool ensure_mqtt() {
    if (modem_client_is_connected()) {
        return true;
    }
    Serial.println("mqtt disconnected, attempting reconnect");
    mqtt.disconnect();
    if (!mqtt.connect(endpoint, user, MQTT_PASSWORD)) {
        Serial.println("mqtt connect failed!");
        return false;
    }
    return true;
}

static void handle_control(char *topic, byte *payload, unsigned int len) {
    if (strcasecmp(topic, topicControl) != 0) {
        return; // Not a control message
    }
    if (payload == nullptr || len == 0) {
        return; // Empty message
    }
    // Parse payload and extract command
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, len);
    if (err || !doc["cmd"].is<const char*>()) {
        Serial.printf("invalid control message json: %s\n", err.c_str());
        return;
    }
    const char *cmd = doc["cmd"];
    Serial.printf("received control command: %s\n", cmd);
    
    // Acknowledge we received the command
    char ackPayload[192];
    snprintf(ackPayload, sizeof(ackPayload), "{\"ok\":true,\"cmd\":\"%s\"}", cmd);
    mqtt.publish(topicAck, ackPayload);
    delay(500); // Ensure ack is sent
    
    // Execute the command
    if (strcasecmp(cmd, "reboot") == 0) {
        ESP.restart();
    } else if (strcasecmp(cmd, "ota") == 0) {
        // Deinit MQTT so we can connect to HTTP
        minimote_comm_deinit();
        // Disable modem PSM to ensure it doesn't interfere with the OTA process
        modem_set_psm(false);
        if (!ota_do_update(doc["server"].as<const char*>(), doc["path"].as<const char*>())) {
            Serial.println("OTA update failed");
            // OTA failed and we will be returning to this current code, so re-enable PSM to save power
            modem_set_psm(true);
            return;
        }
        delay(1000);
        ESP.restart();
    } else if (strcasecmp(cmd, "set_slot") == 0) {
        if (!doc["seconds"].is<int>()) {
            return;
        }
        uint32_t seconds = doc["seconds"];
        minimote_set_slot_seconds(seconds);
        Serial.printf("set slot seconds to %lu\n", seconds);
    } else {
        Serial.println("unknown control command");
        return;
    }
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
    // Connect and subscribe to control topic
    return minimote_comm_subscribe_control();
}

void minimote_comm_deinit() {
    mqtt.disconnect();
}

bool minimote_comm_sync_time() {
    time_t epoch = modem_cell_read_time();
    if (epoch <= 0) {
        Serial.println("failed to sync time");
        return false;
    } else if (epoch < 1700000000) {
        Serial.printf("time synced, but seems wrong? (%ld)\n", epoch);
        return false;
    }

    // Syncronize system clock with NTP time
    timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    Serial.printf("time synced, epoch=%ld\n", epoch);
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
    if (!ensure_mqtt()) {
        return false;
    }
    if (!mqtt.subscribe(topicControl, 1)) {
        Serial.println("failed to subscribe control topic");
        return false;
    }
    return true;
}

bool minimote_comm_unsubscribe_control() {
    if (!ensure_mqtt()) {
        return false;
    }
    if (!mqtt.unsubscribe(topicControl)) {
        Serial.println("failed to unsubscribe control topic");
        return false;
    }
    return true;
}

bool minimote_comm_publish_sample() {
    if (!ensure_mqtt()) {
        return false;
    }

    // Harvest all sensor data
    float battV = pmu_get_battery_voltage();
    int battPct = pmu_get_battery_percent();
    float temperature = temp_read();
    float turbidity = get_turbidity();
    float tds = get_tds();
    float velo = velo_read();
    double ozone = ozone_read();
    EnvData env = env_read();
    PMData pm = pm_read();
    float chamberTemp = chamber_read();
    // Construct JSON payload and publish to MQTT topic
    char payload[512];
    snprintf(payload, sizeof(payload),
        "{\"unix_time\":%ld,\"millis\":%lu,\"battery_v\":%.3f,\"battery_pct\":%d,\"water_temp\":%.2f,\"turbidity\":%.2f,\"tds\":%.2f,\"air_velocity\":%.2f,\"ozone\":%.4f,\"air_temp\":%.2f,\"humidity\":%.2f,\"uv\":%.2f,\"lum\":%.2f,\"baro\":%.2f,\"pm1_0\":%.2f,\"pm2_5\":%.2f,\"pm10\":%.2f,\"chamber_temp\":%.2f}",
        time(nullptr), millis(),
        battV, battPct,
        temperature, turbidity, tds,
        velo, ozone, env.tempC, env.hum, env.uv, env.lum, env.baro,
        pm.pm1_0, pm.pm2_5, pm.pm10, chamberTemp
    );

    bool published = mqtt.publish(topicData, payload);
    Serial.printf("sample publish %s\n", published ? "ok" : "failed");
    return published;
}

bool minimote_comm_publish_init(const char *init) {
    if (!ensure_mqtt()) {
        return false;
    }
    bool published = mqtt.publish(topicInit, init);
    Serial.printf("init publish %s\n", published ? "ok" : "failed");
    return published;
}

#endif // MINIMOTE
