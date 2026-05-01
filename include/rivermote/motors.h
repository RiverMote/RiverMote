#pragma once

#include <stdint.h>

#if RIVERMOTE

/**
 * Initialize motor control.
 */
void motors_init();

/**
 * Set motor speeds.
 * @param motor1 speed for motor 1 (-255..255)
 * @param motor2 speed for motor 2 (-255..255)
 */
void motors_set(int16_t motor1, int16_t motor2);

/**
 * Get the current maximum speed as a fraction of full speed.
 * @return max speed (0.0 to 1.0)
 */
float motors_get_max_speed();

/**
 * Set the maximum speed as a fraction of full speed.
 * @param speed max speed (0.0 to 1.0)
 */
void motors_set_max_speed(float speed);

#endif // RIVERMOTE
