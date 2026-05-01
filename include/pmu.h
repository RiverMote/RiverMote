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
 * Get VBUS (solar/USB) voltage.
 * @return voltage in volts, or -1.0 if VBUS is not connected
 */
float pmu_get_vbus_voltage();

/**
 * @return true if the PMU reports battery is currently charging
 */
bool pmu_is_charging();

/**
 * Enable or disable power to the sensors.
 * @param on true to enable power, false to disable
 */
void pmu_set_sensor_power(bool on);
