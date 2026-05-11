#include "sensors/air_velocity.h"

#if MINIMOTE

#include <DFRobot_PAV3000.h>

DFRobot_PAV3000 velo;
static bool ready = false;

bool velo_init() {
    // return velo.setRange(AIRFLOW_RANGE_15_MPS);
    ready = velo.setRange(AIRFLOW_RANGE_7_MPS) == 1;
    return ready;
}


float velo_read() {
    if (!ready) {
        return -1.f;
    }
    return velo.readMeterPerSec();
    // return velo.readRaw();
    // return velo.readMilePerHour();
}

#endif // MINIMOTE
