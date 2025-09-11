#include <Arduino.h>

#include "modem.h"

#include "mqtt.h"

#define MQTT_BROKER ""
#define MQTT_PORT "1883"
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_CLIENT_ID ""

bool mqtt_init() {
    if (!modem_cell_is_connected()) {
        Serial.println("cell not connected, can't init MQTT");
        return false;
    }

    Serial.println("setting up MQTT configuration");
    // Send a disconnect in case we were connected before
    modem_send("+SMDISC");
    // Set up MQTT configuration
    bool configured = true;
    configured &= modem_send("+SMCONF=\"URL\",\"" MQTT_BROKER "\"," MQTT_PORT);
    configured &= modem_send("+SMCONF=\"USERNAME\",\"" MQTT_USER "\"");
    configured &= modem_send("+SMCONF=\"PASSWORD\",\"" MQTT_PASS "\"");
    configured &= modem_send("+SMCONF=\"CLIENTID\",\"" MQTT_CLIENT_ID "\"");
    if (!configured) {
        Serial.println("failed to set up MQTT configuration");
        return false;
    }

    // Connect to broker
    Serial.print("connecting to MQTT broker (may take a moment)...");
    bool connected = false;
    do {
        connected = modem_send("+SMCONN", 30000);
        if (!connected) {
            Serial.print("failed, retrying...");
            delay(1000);
        }
    } while (!connected);
    Serial.println("connected!");
    return true;
}

bool mqtt_is_connected() {
    return modem_get_smstate();
}

bool mqtt_publish(const char *topic, const char *payload) {
    if (!mqtt_is_connected()) {
        Serial.println("MQTT not connected, can't publish");
        return false;
    }
    // Construct publish command and send it + payload
    char cmd[512];
    // AT+SMPUB=<topic>,<content length>,<qos>,<retain><CR>message is enteredQuit edit mode if messagelength equals to <contentlength>
    snprintf(cmd, sizeof(cmd), "+SMPUB=\"%s\",%d,1,1", topic, strlen(payload));
    return modem_send_smpub(cmd, payload, strlen(payload));
}
