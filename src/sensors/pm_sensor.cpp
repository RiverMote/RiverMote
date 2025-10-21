#include "sensors/pm_sensor.h"

#if MINIMOTE

#include <SparkFun_BMV080_Arduino_Library.h>

SparkFunBMV080 bmv080;

bool pm_init() {
    if (!bmv080.begin()) {
        return false;
    }
    if (!bmv080.init()) {
        return false;
    }
    return bmv080.setMode(SF_BMV080_MODE_CONTINUOUS);
}

PMData pm_read() {
    bmv080.readSensor();
    return PMData{
        .pm10 = bmv080.PM10(),
        .pm2_5 = bmv080.PM25(),
        .pm1_0 = bmv080.PM1()
    };
}

#endif // MINIMOTE
