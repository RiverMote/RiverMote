#include "minimote/power.h"

#if MINIMOTE

#include <Arduino.h>
#include <esp_sleep.h>
#include <time.h>
#include <Wire.h>

#include "minimote/comm.h"
#include "pins.h"
#include "sensors/air_velocity.h"
#include "sensors/chamber.h"
#include "sensors/env.h"
#include "sensors/ozone.h"
#include "sensors/pm_sensor.h"
#include "modem.h"
#include "pmu.h"

#define LOW_BATTERY_VOLT 3.55f // Low power mode will be entered under this voltage (~25% for a healthy 18650 cell)
#define LOW_BATTERY_SLEEP_SEC 7200 // Time to deep sleep when low battery is detected, in seconds (2 hours)
#define DEFAULT_SLOT_SECONDS 900 // Duration to sleep between publish windows, in seconds (15 minutes). Can be changed by sending a control message
#define SLOT_WINDOW_PERCENT 0.1f // Percentage ±of the slot duration to consider "still within the window" (symmetric — applies both before and after the boundary)

static uint32_t slotSec = DEFAULT_SLOT_SECONDS;

// Reinitializes i2c and all sensors connected to it.
static void reinit_i2c() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ);
    env_init();
    ozone_init();
    velo_init();
    pm_init();
    chamber_init();
}

/**
 * Puts the device to sleep for the specified duration.
 * @note If deep is specified, all peripherals will be powered down and the device will be reset upon wakeup.
 */
static void minimote_enter_sleep(uint32_t sleep_sec, bool deep) {
    Serial.printf("%s sleeping for %lu sec\n", deep ? "deep" : "light", (unsigned long)sleep_sec);
    uint64_t sleepUs = (uint64_t)sleep_sec * 1000000ULL;

    // Unsubscribe from control topic as we can't receive during sleep
    minimote_comm_unsubscribe_control();

    // Stop i2c and power down sensors
    Wire.end();
    pmu_set_sensor_power(false);

    if (deep) {
        minimote_comm_deinit();
        modem_deinit(); // Disconnect from cell + fully power down modem
        pmu_set_modem_power(false);
        esp_sleep_enable_timer_wakeup(sleepUs);
        esp_deep_sleep_start();
        // Deep sleep will reset the device, so code execution will not continue past esp_deep_sleep_start()
        // It will pick back up at setup() after waking up
        return;
    }

    // Light sleep path; put modem to sleep and then sleep esp
    modem_set_sleep(true);
    Serial.end();
    esp_sleep_enable_timer_wakeup(sleepUs);
    esp_light_sleep_start();
    Serial.begin(115200);

    // Upon waking up, restore sensor power and wake modem
    pmu_set_sensor_power(true);
    modem_set_sleep(false); // This function takes a while to wake modem, so i2c sensors will be booted by the time it returns

    // Reconnect to i2c sensors
    reinit_i2c();

    // Resubscribe to control topic to receive any commands that were sent while we were asleep
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
        uint32_t offset = (uint32_t)(now % slotSec);
        // If we are within the trailing edge of the window (i.e. we just published and are still
        // inside it), sleep a full slot so we don't immediately re enter the window on the next wake
        uint32_t window = (uint32_t)(slotSec * SLOT_WINDOW_PERCENT);
        uint32_t sleepSec = (offset < window) ? slotSec : (slotSec - offset);
        minimote_enter_sleep(sleepSec, false);
    } else {
        // Invalid time, fallback to a reasonable sleep duration
        minimote_enter_sleep(DEFAULT_SLOT_SECONDS, false);
    }
}

bool minimote_within_publish_window() {
    time_t now = time(nullptr);
    if (now <= 0) {
        // If time is invalid, assume we should publish (better to publish too often than miss data)
        return true;
    }
    uint32_t offset = (uint32_t)(now % slotSec);
    uint32_t window = (uint32_t)(slotSec * SLOT_WINDOW_PERCENT);
    // Symmetric window: publish if within `window` seconds after the boundary,
    // or within `window` seconds before the next boundary
    return offset < window || offset > (slotSec - window);
}

void minimote_set_slot_seconds(uint32_t seconds) {
    slotSec = seconds;
}

#endif // MINIMOTE
