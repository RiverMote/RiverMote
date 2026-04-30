#pragma once

typedef struct EnvData {
    double tempC, tempF, hum, uv, lum, baro;
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
