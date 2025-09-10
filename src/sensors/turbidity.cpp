#include <Arduino.h>

#include "pins.h"

#include "sensors/turbidity.h"

void turbidity_init() {
    pinMode(PIN_TURBIDITY, INPUT);
}

float get_turbidity() {
    int sensorValue = analogRead(PIN_TURBIDITY);
    float voltage = sensorValue * (5.0 / 1024.0); // ref volatge / 2^12
    return voltage;
}