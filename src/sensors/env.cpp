#include "sensors/env.h"

#if MINIMOTE

#include <DFRobot_EnvironmentalSensor.h>

static DFRobot_EnvironmentalSensor environment(/*addr = */SEN050X_DEFAULT_DEVICE_ADDRESS);
static bool ready = false;

bool env_init() {
    ready = environment.begin() == 0;
    return ready;
}

EnvData env_read() {
    if (!ready) {
        return {NAN, NAN, NAN, NAN, NAN};
    }
    EnvData data = {
        .tempC = environment.getTemperature(TEMP_C),
        .hum = environment.getHumidity(),
        .uv = environment.getUltravioletIntensity(eS12SD),
        .lum = environment.getLuminousIntensity(),
        .baro = static_cast<float>(environment.getAtmospherePressure(HPA)),
    };
    if (data.tempC < -40.f || data.tempC > 50.f) {
        data.tempC = NAN;
    }
    if (data.hum < 0.f || data.hum > 100.f) {
        data.hum = NAN;
    }
    return data;
}

#endif // MINIMOTE
