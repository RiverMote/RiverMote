#include "minimote/mqtt.h"

#if MINIMOTE

#include <Arduino.h>
#include <string.h>

#include "modem.h"
#if __has_include("secrets.h")
    #include "secrets.h"
#endif

// The below configuration options can be overridden by defining them in secrets.h or elsewhere.
#ifndef MQTT_BROKER
    #define MQTT_BROKER ""
#endif
#ifndef MQTT_PORT
    #define MQTT_PORT "1883"
#endif
#ifndef MQTT_USE_TLS
    #define MQTT_USE_TLS 0
#endif
#define MQTT_MAX_CONN_RETRIES 3

// Parses a string of the form +SMSUB/SMRECV: "<topic>","<payload>"
static bool parse_quoted_pair(const String &line, String &topic, String &payload) {
    int q1 = line.indexOf('"');
    if (q1 < 0) {
        return false;
    }
    int q2 = line.indexOf('"', q1 + 1);
    if (q2 < 0) {
        return false;
    }
    topic = line.substring(q1 + 1, q2);

    int q3 = line.indexOf('"', q2 + 1);
    if (q3 >= 0) {
        int q4 = line.indexOf('"', q3 + 1);
        if (q4 > q3) {
            payload = line.substring(q3 + 1, q4);
            return true;
        }
    }
    return true;
}

// Decodes a hex-encoded string into its raw form. Returns false if the input is not valid hex.
static bool hex_decode_payload(const String &hex, String &out) {
    int len = hex.length();
    if (len == 0 || (len % 2) != 0) return false;
    out = "";
    out.reserve(len / 2);
    for (int i = 0; i < len; i += 2) {
        char buf[3] = { hex[i], hex[i+1], '\0' };
        char *end;
        long b = strtol(buf, &end, 16);
        if (end != buf + 2) return false;
        out += (char)b;
    }
    return true;
}

bool mqtt_init(const char *client, const char *user, const char *pass) {
    if (!modem_cell_is_connected()) {
        Serial.println("cell not connected, can't init MQTT");
        return false;
    }

    Serial.println("setting up MQTT configuration");
    // Send a disconnect in case we were connected before
    modem_send("+SMDISC");
    // Construct dynamic configuration commands
    char userCmd[192], passCmd[192], clientCmd[192];
    snprintf(userCmd, sizeof(userCmd), "+SMCONF=\"USERNAME\",\"%s\"", user);
    snprintf(passCmd, sizeof(passCmd), "+SMCONF=\"PASSWORD\",\"%s\"", pass);
    snprintf(clientCmd, sizeof(clientCmd), "+SMCONF=\"CLIENTID\",\"%s\"", client);
    // Set up MQTT configuration
    bool configured = true;
    configured &= modem_send("+SMCONF=\"URL\",\"" MQTT_BROKER "\"," MQTT_PORT);
    configured &= modem_send(userCmd);
    configured &= modem_send(passCmd);
    configured &= modem_send(clientCmd);
    configured &= modem_send("+SMCONF=\"SUBHEX\",1"); // Subscribe payloads are hex-encoded for easier parsing
    configured &= modem_send("+SMCONF=\"ASYNCMODE\",1"); // Async mode enabled
    configured &= modem_send("+SMCONF=\"CLEANSS\",1"); // Clean session
#if MQTT_USE_TLS
    configured &= modem_set_mqtt_ssl(true);
#endif
    if (!configured) {
        Serial.println("failed to set up MQTT configuration");
        return false;
    }

    // Connect to broker
    Serial.print("connecting to MQTT broker (may take a moment)...");
    bool connected = false;
    int retries = 0;
    do {
        connected = modem_send("+SMCONN", 30000);
        if (!connected) {
            Serial.print("failed, retrying...");
            delay(1000);
        }
        retries++;
    } while (!connected && retries < MQTT_MAX_CONN_RETRIES);
    if (!connected) {
        Serial.println("failed to connect to MQTT broker");
        return false;
    }
    Serial.println("connected!");
    return true;
}

bool mqtt_is_connected() {
    return modem_mqtt_is_connected();
}

bool mqtt_publish(const char *topic, const char *payload, uint8_t qos, bool retain) {
    if (!mqtt_is_connected()) {
        Serial.println("MQTT not connected, can't publish");
        return false;
    }
    // Construct publish command and send it + payload
    char cmd[1024];
    // AT+SMPUB=<topic>,<content length>,<qos>,<retain><CR>message is enteredQuit edit mode if messagelength equals to <contentlength>
    snprintf(cmd, sizeof(cmd), "+SMPUB=\"%s\",%d,%u,%u", topic, strlen(payload), qos > 1 ? 1 : qos, retain ? 1 : 0);
    return modem_send_smpub(cmd, payload, strlen(payload));
}

bool mqtt_subscribe(const char *topic, uint8_t qos) {
    if (!mqtt_is_connected()) {
        Serial.println("MQTT not connected, can't subscribe");
        return false;
    }
    return modem_send_smsub(topic, qos);
}

bool mqtt_read_message(String &topic, String &payload, uint32_t timeout) {
    topic = "";
    payload = "";
    // Have we received any arbitrary messages from the modem?
    String line;
    if (!modem_read_line(line, timeout)) {
        return false;
    }
    // Does that message start with the expected prefix for inbound MQTT messages?
    if (!(line.startsWith("+SMSUB:") || line.startsWith("+SMRECV:"))) {
        return false;
    }
    // This is an inbound MQTT message; parse out the topic and payload, which are expected to be in quoted strings
    if (!parse_quoted_pair(line, topic, payload) || topic.length() == 0) {
        return false;
    }

    if (payload.length() == 0) {
        // Try another line, which may contain the payload if it wasn't included on the first line for some reason
        String nextLine;
        if (modem_read_line(nextLine, 200)) {
            payload = nextLine;
        }
    }
    if (payload.length() > 0) {
        String decoded;
        if (hex_decode_payload(payload, decoded)) {
            payload = decoded;
        }
    }
    payload.trim();
    return payload.length() > 0;
}

#endif // MINIMOTE
