#pragma once

typedef struct EnvData {
    // °C, %, mw/cm2, lux, hPa
    float tempC, hum, uv, lum, baro;
} EnvData;

/**
 * Initialize the DFR environemental sensor.
 * @return true on successful initialization
 */
bool env_init();

/**
 * @return env sensors values
 */
EnvData env_read();
