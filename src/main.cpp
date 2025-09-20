#include <Arduino.h>
#include <Wire.h>

#include "sensors/imu.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"
#include "sensors/tds.h"
#include "bluetooth.h"
#include "flasher.h"
#include "sensors/light.h"
#include "modem.h"
#include "motors.h"
#include "mqtt.h"
#include "pins.h"
#include "pmu.h"
#include "sd.h"
#include "flasher.h"
#include "debugging.h"

#define WAIT_FOR_SERIAL 0

#define LOG_INTERVAL 500 // ms between log entries
#define LOG_HEADER "Millis,Time,Batt%,BattV,Lat,Lng,Speed,Track,Temp,Turbidity,TDS"

void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL
	while (!Serial);
	delay(1000);
#endif
	Serial.println("Welcome to river mote!");

	// Initialize PMU
	Serial.print("Initializing pmu:");
    if (!pmu_init()) {
		Serial.println("! pmu init failed! halting.");
		while (true) yield();
	}
	Serial.println("- pmu initialized");

	// Initialize modem
	Serial.print("Initializing modem:");
	if (modem_init()) {
		Serial.println("- modem initialized");
	} else {
		Serial.println("! modem init failed!");
	}

#if RIVERMOTE
	// GPS
	Serial.println("enabling modem gps");
	if (modem_gps_enable()) {
		Serial.println("modem gps enabled");
	} else {
		Serial.println("modem gps enable failed!");
	}
#endif
#if MINIMOTE
	// Celluar
	Serial.println("enabling modem cellular");
	if (modem_cell_enable()) {
		Serial.println("modem cellular enabled");
	} else {
		Serial.println("modem cellular enable failed!");
	}
#endif

	// Initialize sensors
	Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ);
	// light sensor
	init_cds();
	// Flasher
	flasher_init();
	// Temperature sensor
	Serial.print("Initializing temperature sensor:");
	if (temp_init()) {
		Serial.println("- temperature sensor initialized");
	} else {
		Serial.println("! temperature sensor init failed!");
	}
	// IMU
	Serial.print("Initializing imu:");
	if (imu_init()) {
		Serial.println("- imu initialized");
	} else {
		Serial.println("! imu init failed!");
	}

#if RIVERMOTE
	// Bluetooth
	Serial.println("Initializing bluetooth:");
	if (bluetooth_init()) {
		Serial.println("- bluetooth initialized");
	} else {
		Serial.println("! bluetooth init failed!");
	}

	// Motors
	Serial.println("Initializing motors:");
	motors_init();
	Serial.println("- motors initialized");

	// SD card
	Serial.println("Initializing sd card:");
	if (sd_init()) {
		Serial.println("- sd initialized");
	} else {
		Serial.println("! sd init failed!");
	}
	// Create new data file
	sd_create_new(LOG_HEADER);
#endif // RIVERMOTE
#if MINIMOTE
	// MQTT
	Serial.println("Initializing mqtt:");
	if (mqtt_init()) {
		Serial.println("- mqtt initialized");
	} else {
		Serial.println("! mqtt init failed!");
	}
#endif

	Serial.println("Ready!");
}

/*
long loopMillis;
double sleepMinutes = 0.5;
*/

void loop() {
	flash_beacon(); // beacon light
#if RIVERMOTE
	// Bluetooth motor control
	uint8_t buttons = bluetooth_get_pressed();
	if (buttons & 0b00001000) { // up
		motors_set(255, 255);
	} else if (buttons & 0b00000100) { // down
		motors_set(-255, -255);
	} else if (buttons & 0b00000010) { // left
		motors_set(-255, 255);
	} else if (buttons & 0b00000001) { // right
		motors_set(255, -255);
	} else {
		motors_set(0, 0);
	}

	// If enough time has passed from last log...
	static unsigned long lastLog = 0;
	if (millis() - lastLog > LOG_INTERVAL) {
		// Read sensors
		String time = modem_gps_read_time();
		ModemGPSData gps = modem_gps_read();
		float temperature = temp_read();
		float turbidity = get_turbidity();
		float tds = get_tds();
		// Log to sd and bluetooth
		sd_appendf("%s,%d,%.3f,%.6f,%.6f,%.2f,%.2f,%.2f,%.4f,%.4f", 
			millis(), time.c_str(), pmu_get_battery_percent(), pmu_get_battery_voltage(),
			gps.lat, gps.lng, gps.speed, gps.track,
			temperature, turbidity, tds);
		bluetooth_printf("%.3fV %d%%\n", pmu_get_battery_voltage(), pmu_get_battery_percent());
		lastLog = millis();
	}
#endif

	/*
	if (millis() - loopMillis > (sleepMinutes* 60000)) {
		debug_reporter();
		loopMillis = millis();
    }
	*/

	/*
	int ll = check_cds(); // get light level
	if(ll < 2500) {
		Serial.println("Going to sleep now");
  		delay(1000);
	  	Serial.flush(); 
  		esp_deep_sleep_start();
	}
	*/

	delay(50);
}
