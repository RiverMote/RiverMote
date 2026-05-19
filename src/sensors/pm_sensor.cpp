#include "sensors/pm_sensor.h"

#if MINIMOTE

#include <SparkFun_BMV080_Arduino_Library.h>

static SparkFunBMV080 bmv080;
static bool ready = false;

bool pm_init() {
    if (!bmv080.begin()) {
        return false;
    }
    if (!bmv080.init()) {
        return false;
    }
    ready = bmv080.setMode(SF_BMV080_MODE_CONTINUOUS);
    return ready;
}

PMData pm_read() {
    if (!ready) {
        return {};
    }
    bmv080.readSensor();
    return PMData{
        .pm10 = bmv080.PM10(),
        .pm2_5 = bmv080.PM25(),
        .pm1_0 = bmv080.PM1()
    };
}

#endif // MINIMOTE
