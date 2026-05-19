#pragma once

#include <Arduino.h>
#include <time.h>
#include <TinyGsmClient.h>

#define LOG_MODEM 0 // Duplicate all modem communications to Serial for debugging

typedef struct ModemGPSData {
    double lat, lng;
    float speed;
    float track;
} ModemGPSData;

/* -- General modem functions -- */

/**
 * Initialize the modem with the specified baud rate and maximum retries to establish communication.
 * @param baud baud rate for modem serial communication
 * @param max_retries maximum number of retries to establish communication
 * @return true if the modem was successfully initialized
 */
bool modem_init(unsigned long baud = 115200, uint max_retries = 10);

/**
 * @return a reference to the underlying modem instance.
 */
TinyGsm& modem_get();

/**
 * Send an arbitrary AT command to the modem and wait for a response.
 * @param cmd AT command to send (e.g. "+CSQ")
 * @param timeout time in milliseconds to wait for a response
 * @return true if a successful response was received within the timeout period
 */
bool modem_send(const char *cmd, uint32_t timeout = 1000);

/**
 * Put modem in/out of light sleep mode.
 * @param enable true to begin sleep, false to wake up
 * @note The modem cannot receive or send data while asleep, but will maintain any active connections.
 */
void modem_set_sleep(bool enable);

/**
 * Completely deinitialize the modem, including any active connections.
 * After this, the modem will need to be reinitialized with `modem_init()` before it can be used again.
 */
void modem_deinit();

/* -- GPS functions -- */

/**
 * Enable the modem's GPS.
 * @return true if GPS was successfully enabled
 * @note GPS cannot be enabled if cellular connection is active
 */
bool modem_gps_enable();

/**
 * @return true if GPS is currently enabled
 */
bool modem_gps_is_enabled();

/**
 * Read the current GPS data from the modem.
 * @return ModemGPSData structure, or all fields zeroed if reading failed
 */
ModemGPSData modem_gps_read();

/**
 * Read the current GPS time from the modem.
 * @return String containing the current time in format `yyyyMMddhhmmss`, or "00000000000000" if GPS is not fixed
 */
String modem_gps_read_time();

/**
 * @return true if the GPS has a valid fix
 */
bool modem_gps_fixed();

/* -- Cellular functions -- */

/**
 * Enable the modem's cellular connection.
 * @return true if cellular connection was successfully enabled
 * @note Cellular connection cannot be enabled if GPS is active
 */
bool modem_cell_enable();

/**
 * @return true if cellular connection is currently enabled and connected
 */
bool modem_cell_is_connected();

/**
 * @return current network time (UTC) as epoch timestamp, or 0 if reading failed
 */
time_t modem_cell_read_time();

/* -- SM (MQTT) functions -- */

/**
 * Get the current MQTT connection status.
 * @return true if connected
 */
bool modem_mqtt_is_connected();

/**
 * Enable or disable TLS for the modem's MQTT session.
 * @param enable true to enable TLS, false to disable
 * @return true if the modem accepted the configuration
 */
bool modem_set_mqtt_ssl(bool enable);

/**
 * Send a +SMPUB command to publish an MQTT message.
 * @param cmd full +SMPUB command string (e.g. +SMPUB=...)
 * @param payload MQTT message payload
 * @param payload_len length of the payload
 * @return true if the message was successfully published
 */
bool modem_send_smpub(const char *cmd, const char *payload, size_t payload_len);

/**
 * Subscribe to an MQTT topic.
 * @param topic MQTT topic
 * @param qos quality of service (0 or 1)
 * @return true if the modem accepted the command
 */
bool modem_send_smsub(const char *topic, uint8_t qos = 1);

/**
 * Unsubscribe from an MQTT topic.
 * @param topic MQTT topic
 * @return true if the modem accepted the command
 */
bool modem_send_smunsub(const char *topic);

/**
 * Read one line from modem stream, including unsolicited MQTT indications.
 * @param line output line, without trailing CR/LF
 * @param timeout time in milliseconds to wait for data
 * @return true if a non-empty line was read
 */
bool modem_read_line(String &line, uint32_t timeout = 0);
