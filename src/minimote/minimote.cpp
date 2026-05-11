#include "minimote/minimote.h"

#if MINIMOTE

#include <Arduino.h>
#include <time.h>

#include "minimote/comm.h"
#include "minimote/power.h"
#include "modem.h"

// How long to keep the control window open after a publish
// (how long we spend with the modem on and waiting for a control message)
// This exists as a grace period to allow for the modem to process incoming messages
#define CONTROL_WINDOW_MS 5000

void minimote_init(JsonDocument &initted) {
    Serial.println("enabling modem cellular");
    if (modem_cell_enable()) {
        Serial.println("- modem cellular enabled");
    } else {
        Serial.println("! modem cellular enable failed!");
        return;
    }

    Serial.println("establishing communication");
    if (!minimote_comm_init()) {
        Serial.println("! communication init failed!");
        return;
    }
    if (!minimote_comm_sync_time()) {
        Serial.println("! time sync failed, will rely on fallback time");
    }
    Serial.println("- communication established");

    // Report back on which sensors were initialized successfully
    initted["unix_time"] = time(nullptr);
    char inittedStr[1024];
    serializeJson(initted, inittedStr);
    minimote_comm_publish_init(inittedStr);
}

void minimote_tick() {
    if (minimote_within_publish_window()) {
        // We are within the specified time to publish a sample, so do it and open the control window afterwards
        minimote_comm_publish_sample();
        minimote_comm_control_window(CONTROL_WINDOW_MS);
    }
    minimote_manage_power();
}

#endif // MINIMOTE
