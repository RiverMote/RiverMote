#include <OneWire.h>
#include <DallasTemperature.h>

#include "pins.h"

#include "sensors/temp.h"

OneWire oneWire(PIN_TEMP);
DallasTemperature temp(&oneWire);
static bool conversionPending = false;
static float lastTemp = 0.f;

bool temp_init() {
    temp.begin();
    temp.setWaitForConversion(false); // Set non-blocking
    uint8_t devices = temp.getDeviceCount();
    Serial.printf("found %d temperature devices\n", devices);
    return devices > 0;
}

float temp_read() {
    if (temp.isConversionComplete()) {
        lastTemp = temp.getTempCByIndex(0);
        conversionPending = false;
    } else if (!conversionPending) {
        temp.requestTemperatures();
        conversionPending = true;
    }
    return lastTemp;
}
