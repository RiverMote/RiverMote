#pragma once

#if RIVERMOTE

#include <stdint.h>

typedef struct Waypoint {
    double lat, lng;
} Waypoint;

/**
 * Initialize control system.
 */
void control_init();

/**
 * Handle manual input control mode.
 * @param buttons bitfield of buttons currently pressed (from `bluetooth_get_pressed()`)
 */
void control_handle_input(uint8_t buttons);

/**
 * Handle autonomous control mode.
 * Should be called periodically to perform autonomous navigration.
 */
void control_autonomous_mode();

/**
 * @return true if the current control is in autonomous mode
 */
bool control_is_autonomous();

#endif // RIVERMOTE
