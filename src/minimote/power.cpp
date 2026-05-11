#include "minimote/power.h"

#if MINIMOTE

#include <Arduino.h>
#include <esp_sleep.h>
#include <time.h>

#include "minimote/comm.h"
#include "modem.h"
#include "pmu.h"

#define LOW_BATTERY_VOLT 3.55f // Low power mode will be entered under this voltage (~25% for a healthy 18650 cell)
#define LOW_BATTERY_SLEEP_SEC 3600 // Time to deep sleep when low battery is detected, in seconds (1 hour)
#define SLOT_SECONDS 900 // Duration to sleep between publish windows, in seconds (15 minutes)
#define SLOT_WINDOW_SEC 120 // How long after the exact publish time slot to consider "within the publish window", in seconds (2 minutes)
#define FALLBACK_SLEEP_SEC 300 // Fallback sleep duration when time is invalid, in seconds (5 minutes)

/**
 * Puts the device to sleep for the specified duration.
 * @note If deep is specified, all peripherals will be powered down and the device will be reset upon wakeup.
 */
static void minimote_enter_sleep(uint32_t sleep_sec, bool deep) {
    Serial.printf("%s sleeping for %lu sec\n", deep ? "deep" : "light", (unsigned long)sleep_sec);
    minimote_comm_unsubscribe_control();
    uint64_t sleepUs = (uint64_t)sleep_sec * 1000000ULL;
    if (deep) {
        modem_send("+SMDISC"); // Disconnect from MQTT
        modem_deinit(); // Disconnect from cell + fully power down modem
        pmu_set_sensor_power(false); // Cut power to sensors
        esp_sleep_enable_timer_wakeup(sleepUs);
        esp_deep_sleep_start();
        // Deep sleep will reset the device, so code execution will not continue past esp_deep_sleep_start()
        // It will pick back up at setup() after waking up
        return;
    }
    // Light sleep path
    modem_set_sleep(true);
    Serial.end();
    esp_sleep_enable_timer_wakeup(sleepUs);
    esp_light_sleep_start();
    Serial.begin(115200);
    modem_set_sleep(false);
    minimote_comm_subscribe_control();
}

void minimote_manage_power() {
    float battV = pmu_get_battery_voltage();
    // If battery voltage is very low, power down as much as we can to conserve energy
    if (battV > 0.f && battV < LOW_BATTERY_VOLT) {
        Serial.printf("battery low (%.3fV), entering long sleep\n", battV);
        minimote_enter_sleep(LOW_BATTERY_SLEEP_SEC, true);
    }
    // Otherwise, calculate how long to sleep until the next publish window
    time_t now = time(nullptr);
    if (now > 0) {
        uint32_t offset = (uint32_t)(now % SLOT_SECONDS);
        minimote_enter_sleep(offset == 0 ? SLOT_SECONDS : (SLOT_SECONDS - offset), false);
    } else {
        // Invalid time, fallback to a reasonable sleep duration
        minimote_enter_sleep(FALLBACK_SLEEP_SEC, false);
    }
}

bool minimote_within_publish_window() {
    time_t now = time(nullptr);
    if (now <= 0) {
        return true;
    }
    return (uint32_t)(now % SLOT_SECONDS) < SLOT_WINDOW_SEC;
}

#endif // MINIMOTE
