#include "sensors/pm_sensor.h"

#if MINIMOTE

#if defined(NO_BMV080)

bool pm_init() {
    return false;
}

PMData pm_read() {
    return {};
}

#else

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

#endif // NO_BMV080

#endif // MINIMOTE
