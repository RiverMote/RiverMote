#pragma once

#if MINIMOTE

#include <Arduino.h>

/**
 * Initialize the MQTT connection.
 * @param client MQTT client identifier
 * @param user MQTT username
 * @param pass MQTT password
 * @return true if MQTT was successfully initialized and connected
 * @note Strings can be empty, but must not be nullptr
 */
bool mqtt_init(const char *client, const char *user, const char *pass);

/**
 * Set MQTT client identity and credentials.
 * @param client_id client identifier sent to broker
 * @param username broker username
 * @param password broker password
 */
void mqtt_set_identity(const char *client_id, const char *username, const char *password);

/**
 * @return true if MQTT is currently connected
 */
bool mqtt_is_connected();

/**
 * Publish a message to the given topic.
 * @param topic the MQTT topic to publish to
 * @param payload the message payload
 * @return true if the message was successfully published
 */
bool mqtt_publish(const char *topic, const char *payload, uint8_t qos = 1, bool retain = true);

/**
 * Subscribe to the given topic.
 * @param topic MQTT topic to subscribe to
 * @param qos quality of service (0 or 1)
 * @return true if the subscribe command succeeded
 */
bool mqtt_subscribe(const char *topic, uint8_t qos = 1);

/**
 * Read one inbound MQTT message from modem stream.
 * @param topic output topic string
 * @param payload output payload string
 * @param timeout_ms timeout waiting for a message (in ms)
 * @return true if a message was parsed
 */
bool mqtt_read_message(String &topic, String &payload, uint32_t timeout = 0);

#endif // MINIMOTE
