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
 * @return true if MQTT is currently connected
 */
bool mqtt_is_connected();

/**
 * Disconnect from MQTT broker.
 * @return true if successfully disconnected
 */
bool mqtt_deinit();

/**
 * Publish a message to the given topic.
 * @param topic the MQTT topic to publish to
 * @param payload the message payload
 * @return true if the message was successfully published
 */
bool mqtt_publish(const char *topic, const char *payload, uint8_t qos = 0, bool retain = false);

/**
 * Subscribe to the given topic.
 * @param topic MQTT topic to subscribe to
 * @param qos quality of service (0 or 1)
 * @return true if the subscribe command succeeded
 */
bool mqtt_subscribe(const char *topic, uint8_t qos = 1);

/**
 * Unsubscribe from the given topic.
 * @param topic MQTT topic to unsubscribe from
 * @return true if the unsubscribe command succeeded
 */
bool mqtt_unsubscribe(const char *topic);

/**
 * Read one inbound MQTT message from modem stream.
 * @param topic output topic string
 * @param payload output payload string
 * @param timeout_ms timeout waiting for a message (in ms)
 * @return true if a message was parsed
 */
bool mqtt_read_message(String &topic, String &payload, uint32_t timeout = 0);

#endif // MINIMOTE
