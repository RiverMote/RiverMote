#pragma once

/**
 * Initialize the MQTT connection.
 * @return true if MQTT was successfully initialized and connected
 */
bool mqtt_init();

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
bool mqtt_publish(const char *topic, const char *payload);
