#pragma once

/**
 * Initialize the PMU (power management unit).
 * @return true if initialization was successful
 */
bool pmu_init();

/**
 * Get the current battery percentage.
 * @return battery percentage (0-100) or -1 if battery is not connected
 */
int pmu_get_battery_percent();

/**
 * @return current battery voltage in volts, or -1.0 if battery is not connected
 */
float pmu_get_battery_voltage();

/**
 * Enable or disable power to the sensors.
 * @param dc5 true to enable power to the dc5 bus
 * @param dc1 true to enable power to the dc1 bus
 */
void pmu_set_sensor_power(bool dc5, bool dc1);

/**
 * Enable or disable power to the modem.
 * @param on true to enable power, false to disable
 */
void pmu_set_modem_power(bool on);
