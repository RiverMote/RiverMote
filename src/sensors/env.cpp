#include "sensors/env.h"

#if MINIMOTE

#include <DFRobot_EnvironmentalSensor.h>

DFRobot_EnvironmentalSensor environment(/*addr = */SEN050X_DEFAULT_DEVICE_ADDRESS);
static bool ready = false;

bool env_init() {
    ready = environment.begin() == 0;
    return ready;
}

EnvData env_read() {
    if (!ready) {
        return {};
    }
    return (EnvData){
        .tempC = environment.getTemperature(TEMP_C),
        .tempF = environment.getTemperature(TEMP_F),
        .hum = environment.getHumidity(),
        .uv = environment.getUltravioletIntensity(eS12SD),
        .lum = environment.getLuminousIntensity(),
        .baro = static_cast<double>(environment.getAtmospherePressure(HPA)),
    };
}

#endif // MINIMOTE
