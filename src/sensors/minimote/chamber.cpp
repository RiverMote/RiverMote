#include "sensors/minimote/chamber.h"

#if MINIMOTE

#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;

static bool ready = false;

bool chamber_init() {
    ready = aht.begin();

    aht_temp = aht.getTemperatureSensor();
    //aht_temp->printSensorDetails();

    //aht_humidity = aht.getHumiditySensor();
    //aht_humidity->printSensorDetails();

    return ready;
}

float chamber_read() {
    sensors_event_t temp;
    //aht_humidity->getEvent(&humidity);
    aht_temp->getEvent(&temp);
    return temp.temperature;
}

/*
#include <SparkFun_TMP117.h>

static TMP117 temp;
static bool ready = false;

bool chamber_init() {
    ready = temp.begin();
    return ready;
}

float chamber_read() {
    if (!ready || !temp.dataReady()) {
        return NAN;
    }
    float rawTemp = temp.readTempC();
    if (rawTemp < -40.f || rawTemp > 50.f) {
        // Invalid temperature reading, likely due to a sensor error; return NAN to indicate this
        return NAN;
    }
    return rawTemp;
}
*/

#endif // MINIMOTE
