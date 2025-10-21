#pragma once

#include <Arduino.h>

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
 * Send an arbitrary AT command to the modem and wait for a response.
 * @param cmd AT command to send (e.g. "+CSQ")
 * @param timeout time in milliseconds to wait for a response
 * @return true if a successful response was received within the timeout period
 */
bool modem_send(const char *cmd, uint32_t timeout = 1000);

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
 * Disable the modem's GPS.
 * @return true if GPS was successfully disabled
 */
bool modem_gps_disable();

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
 * Disable the modem's cellular connection.
 * @return true if cellular connection was successfully disabled
 */
bool modem_cell_disable();

/* -- SM (MQTT) functions -- */

/**
 * Get the current +SMSTATE (aka MQTT) connection status.
 * @return true if connected
 */
bool modem_get_smstate();

/**
 * Send a +SMPUB command to publish an MQTT message.
 * @param cmd full +SMPUB command string (e.g. +SMPUB=...)
 * @param payload MQTT message payload
 * @param payload_len length of the payload
 * @return true if the message was successfully published
 */
bool modem_send_smpub(const char *cmd, const char *payload, size_t payload_len);
