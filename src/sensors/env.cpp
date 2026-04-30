#include "DFRobot_EnvironmentalSensor.h"

#include "sensors/env.h"

//DFRobot_EnvironmentalSensor environment(/*addr = */SEN050X_DEFAULT_DEVICE_ADDRESS, /*pWire = */&Wire);
DFRobot_EnvironmentalSensor environment(/*addr = */SEN050X_DEFAULT_DEVICE_ADDRESS);

bool env_init() {
    if(environment.begin() != 0){
        //Serial.println(" Sensor initialize failed!!");
        return false;
     } else {
        //Serial.println(" Sensor initialize success!!");
        return true;
     }
    
}

EnvData env_read() {
    return (EnvData){
        .tempC = environment.getTemperature(TEMP_C),
        .tempF = environment.getTemperature(TEMP_F),
        .hum = environment.getHumidity(),
        .uv = environment.getUltravioletIntensity(eS12SD),
        .lum = environment.getLuminousIntensity(),
        .baro = environment.getAtmospherePressure(HPA),
    };
}
