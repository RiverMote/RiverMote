#pragma once

#if MINIMOTE

#include <stdint.h>

/**
 * Initializes communication with the server.
 * @return true if communication was established
 */
bool minimote_comm_init();

/**
 * Syncs the system time with NTP time from the modem.
 * @return true if time was successfully synced or is already recent, false if there was an error syncing time
 */
bool minimote_comm_sync_time();

/**
 * Opens a control window for the specified duration, during which control messages from the server will be read and executed.
 * @param window duration of the control window in milliseconds
 * @note This function will block for the duration of the control window.
 */
void minimote_comm_control_window(uint32_t window);

/**
 * Publishes a sample of all sensor data to the server.
 * @return true if the sample was successfully published
 */
bool minimote_comm_publish_sample();

#endif // MINIMOTE
