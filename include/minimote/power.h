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
 * @param sample number of seconds between each sample slot to set (or 0 to leave unchanged)
 * @param publish number of seconds between each publish slot to set (or 0 to leave unchanged)
 */
void minimote_set_slot_seconds(uint32_t sample, uint32_t publish);

#endif // MINIMOTE
