#include <OneWire.h>
#include <DallasTemperature.h>

#include "pins.h"

#include "sensors/temp.h"

OneWire oneWire(PIN_TEMP);
DallasTemperature temp(&oneWire);
static unsigned long requestedAt = 0; // Time when temperature conversion was requested
static float lastTemp = 0.f;

bool temp_init() {
    temp.begin();
    temp.setWaitForConversion(false); // Set non-blocking
    uint8_t devices = temp.getDeviceCount();
    Serial.printf("found %d temperature devices\n", devices);
    if (devices > 0) {
        temp.requestTemperatures(); // Request initial temperature reading
        requestedAt = millis();
    }
    return devices > 0;
}

float temp_read() {
    if (temp.getDeviceCount() == 0) {
        return 0.f; // No devices to read from
    }

    if (millis() - requestedAt > temp.millisToWaitForConversion()) {
        lastTemp = temp.getTempCByIndex(0);
        temp.requestTemperatures();
        requestedAt = millis();
    }
    return lastTemp;
}
