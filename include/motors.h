#pragma once

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
