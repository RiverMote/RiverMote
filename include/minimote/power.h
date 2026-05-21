#pragma once

#include <stdint.h>

#if MINIMOTE

/**
 * Manages power on the device by powering down peripherals and sleeping based on current power conditions and timing.
 */
void minimote_manage_power();

/**
 * @return true if the current time is within the publish window
 */
bool minimote_within_publish_window();

/**
 * @param seconds the number of seconds between each publish slot to set
 */
void minimote_set_slot_seconds(uint32_t seconds);

#endif // MINIMOTE
