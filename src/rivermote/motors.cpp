#include "rivermote/motors.h"

#if RIVERMOTE

#include <Arduino.h>

#include "rivermote/bluetooth.h"
#include "pins.h"

#define MOTORS_FREQ 20000 // PWM frequency in Hz
#define MOTORS_RES 8 // PWM resolution in bits
#define CHAN_MOTOR1_FWD 0
#define CHAN_MOTOR1_REV 1
#define CHAN_MOTOR2_FWD 2
#define CHAN_MOTOR2_REV 3

static float maxSpeed = 0.f; // Max speed as fraction of full (0.0 to 1.0)

void motors_init() {
    // Create 4 PWM channels for 2 motors (forward and reverse for each)
    bool success = true;
    success &= ledcAttachChannel(PIN_MOTOR1_FWD, MOTORS_FREQ, MOTORS_RES, CHAN_MOTOR1_FWD);
    success &= ledcAttachChannel(PIN_MOTOR1_REV, MOTORS_FREQ, MOTORS_RES, CHAN_MOTOR1_REV);
    success &= ledcAttachChannel(PIN_MOTOR2_FWD, MOTORS_FREQ, MOTORS_RES, CHAN_MOTOR2_FWD);
    success &= ledcAttachChannel(PIN_MOTOR2_REV, MOTORS_FREQ, MOTORS_RES, CHAN_MOTOR2_REV);
    if (!success) {
        return;
    }

    // Enable motors
    pinMode(PIN_MOTOR1_EN, OUTPUT);
    pinMode(PIN_MOTOR2_EN, OUTPUT);
    motors_set(0, 0);
    digitalWrite(PIN_MOTOR1_EN, HIGH);
    digitalWrite(PIN_MOTOR2_EN, HIGH);
}

void motors_set(int16_t motor1, int16_t motor2) {
    // Take the absolute value of each motor speed and constrain to MOTORS_RES
    uint8_t speed1 = constrain(abs(motor1), 0, 255 * maxSpeed);
    uint8_t speed2 = constrain(abs(motor2), 0, 255 * maxSpeed);

    // Motor 1
    if (motor1 > 0) {
        // Positive; forward at given speed
        ledcWrite(PIN_MOTOR1_FWD, speed1);
        ledcWrite(PIN_MOTOR1_REV, 0);
    } else if (motor1 < 0) {
        // Negative; reverse at given speed
        ledcWrite(PIN_MOTOR1_FWD, 0);
        ledcWrite(PIN_MOTOR1_REV, speed1);
    } else {
        // Zero; stop
        ledcWrite(PIN_MOTOR1_FWD, 0);
        ledcWrite(PIN_MOTOR1_REV, 0);
    }
    // Motor 2
    if (motor2 > 0) {
        ledcWrite(PIN_MOTOR2_FWD, speed2);
        ledcWrite(PIN_MOTOR2_REV, 0);
    } else if (motor2 < 0) {
        ledcWrite(PIN_MOTOR2_FWD, 0);
        ledcWrite(PIN_MOTOR2_REV, speed2);
    } else {
        ledcWrite(PIN_MOTOR2_FWD, 0);
        ledcWrite(PIN_MOTOR2_REV, 0);
    }
}

float motors_get_max_speed() {
    return maxSpeed;
}

void motors_set_max_speed(float speed) {
    maxSpeed = constrain(speed, 0.0f, 1.0f);
}

#endif // RIVERMOTE
