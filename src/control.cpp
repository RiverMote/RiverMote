#include "bluetooth.h"
#include "motors.h"

#include "control.h"

#if RIVERMOTE
bool button1Last = false;
bool button2Last = false;
#endif

void control_handle_input(uint8_t buttons) {
#if RIVERMOTE
    // Set motor direction based on buttons pressed
    if (buttons & 0b00001000) { // up
		motors_set(255, 255);
	} else if (buttons & 0b00000100) { // down
		motors_set(-255, -255);
	} else if (buttons & 0b00000010) { // left
		motors_set(-255, 255);
	} else if (buttons & 0b00000001) { // right
		motors_set(255, -255);
	} else {
		motors_set(0, 0);
	}

	// Set motor max speed based on buttons 1 and 2
	if (buttons & 0b01000000) { // button 2
		if (!button2Last) {
			motors_set_max_speed(motors_get_max_speed() + 0.1f); // Increase speed
		}
        button2Last = true;
	} else {
		button2Last = false;
	}
	if (buttons & 0b10000000) { // button 1
		if (!button1Last) {
			motors_set_max_speed(motors_get_max_speed() - 0.1f); // Decrease speed
		}
        button1Last = true;
	} else {
		button1Last = false;
	}
#endif
}

// This is also where autonomous control will go in the future...
