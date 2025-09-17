#include <Arduino.h>
#include <pins.h>

void init() {

}

int read_leaks() {
    int bit_one = digitalRead(PIN_H2O1);
    int bit_two = digitalRead(PIN_H2O2);
    int bit_three = digitalRead(PIN_H2O3);

    int binary_count = bit_one + bit_two << 1 + bit_three << 2;

    return binary_count;
}