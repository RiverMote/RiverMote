#pragma once

#if MINIMOTE

/**
 * Manages power on the device by powering down peripherals and sleeping based on current power conditions and timing.
 */
void minimote_manage_power();

/**
 * @return true if the current time is within the publish window
 */
bool minimote_within_publish_window();

#endif // MINIMOTE
