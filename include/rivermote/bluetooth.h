#pragma once

#if RIVERMOTE

#include <stdint.h>

#define LOG_BLUETOOTH 0 // Duplicate all sent and recieved bluetooth data to serial for debugging

// Button masks for the Bluetooth Control Pad
typedef enum BluetoothButton {
	BTN_1 = 0b10000000, // Decrease speed
	BTN_2 = 0b01000000, // Increase speed
	BTN_3 = 0b00100000, // Toggle autonomous mode
	BTN_4 = 0b00010000, // Unused
	BTN_UP = 0b00001000,
	BTN_DOWN = 0b00000100,
	BTN_LEFT = 0b00000010,
	BTN_RIGHT = 0b00000001,
} BluetoothButton;

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

#endif // RIVERMOTE
