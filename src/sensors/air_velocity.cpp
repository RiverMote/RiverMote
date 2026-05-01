#include "sensors/air_velocity.h"

#if MINIMOTE

#include <DFRobot_PAV3000.h>

DFRobot_PAV3000 velo;

bool velo_init() {
    // return velo.setRange(AIRFLOW_RANGE_15_MPS);
    return velo.setRange(AIRFLOW_RANGE_7_MPS);
}


float velo_read() {
    return velo.readMeterPerSec();
    // return velo.readRaw();
    // return velo.readMilePerHour();
}

#endif // MINIMOTE
