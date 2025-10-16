#pragma once

#include <stdint.h>

#define LOG_BLUETOOTH 0 // Duplicate all sent and recieved bluetooth data to serial for debugging

/**
 * Initialize BLE control (connect via Adafruit Bluefruit).
 * @return true on success
 */
bool bluetooth_init();

/**
 * Get the current pressed buttons bitmask from the Control Pad.
 * Bits 0..7 correspond to buttons 1..8 (e.g. 5=up, 6=down, 7=left, 8=right).
 * @return 8-bit bitmask where 1 means pressed
 */
uint8_t bluetooth_get_pressed();

/**
 * Send a printf-style formatted string via BLE notification to the connected device.
 * @return true on success
 */
bool bluetooth_printf(const char *format, ...);
