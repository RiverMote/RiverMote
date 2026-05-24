#pragma once

#if MINIMOTE

#include <stdint.h>

/**
 * Initializes communication with the server.
 * @return true if communication was established
 */
bool minimote_comm_init();

/**
 * Deinitializes communication with the server.
 */
void minimote_comm_deinit();

/**
 * Syncs the system time with NTP time from the modem.
 * @return true if time was successfully synced
 */
bool minimote_comm_sync_time();

/**
 * Opens a control window for the specified duration, during which control messages from the server will be read and executed.
 * @param window duration of the control window in milliseconds
 * @note This function will block for the duration of the control window.
 */
void minimote_comm_control_window(uint32_t window);

/**
 * Subscribe to the control topic so the device can receive commands.
 * @return true if the subscribe command succeeded or was already subscribed
 */
bool minimote_comm_subscribe_control();

/**
 * Unsubscribe from the control topic before sleeping.
 * @return true if the unsubscribe command succeeded or was already unsubscribed
 */
bool minimote_comm_unsubscribe_control();

/**
 * Publishes a sample of all sensor data to the server.
 * @return true if the sample was successfully published
 */
bool minimote_comm_publish_sample();

/**
 * Publishes initialization information to the server.
 * @param init initialization information JSON string
 * @return true if the init message was successfully published
 */
bool minimote_comm_publish_init(const char *init);

#endif // MINIMOTE
