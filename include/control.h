#pragma once

#include <stdint.h>

/**
 * Handle manual input control mode.
 * @param buttons bitfield of buttons currently pressed (from `bluetooth_get_pressed()`)
 */
void control_handle_input(uint8_t buttons);
