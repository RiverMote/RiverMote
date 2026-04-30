#include <Arduino.h>
#include <Wire.h>

#include "sensors/air_velocity.h"
//#include "sensors/compass.h"
#include "sensors/env.h"
//#include "sensors/imu.h"
//#include "sensors/light.h"
#include "sensors/ozone.h"
#include "sensors/pm_sensor.h"
//#include "sensors/spectral.h"
#include "sensors/tds.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"

#include "bluetooth.h"
#include "control.h"
//#include "flasher.h"
#include "modem.h"
#include "motors.h"
#include "mqtt.h"
#include "pins.h"
#include "pmu.h"
#include "sd.h"
#include "debugging.h"

#define WAIT_FOR_SERIAL 1

#define LOG_INTERVAL 1000 // ms between log entries
#define LOG_HEADER "Millis,Time,Batt%,BattV,Lat,Lng,Speed,Track,Heading,Temp,Turbidity,TDS,SpecViolet,SpecBlue,SpecGreen,SpecYellow,SpecOrange,SpecRed"

void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL
	while (!Serial);
	delay(5000);
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
	// light sensor
	init_cds();
	// Flasher
	flasher_init();
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
	
	// Temperature sensor
	Serial.print("Initializing temperature sensor:");
	if (temp_init()) {
		Serial.println("- temperature sensor initialized");
	} else {
		Serial.println("! temperature sensor init failed!");
	}
	/*
	// Compass
	Serial.print("Initializing compass:");
	if (compass_init()) {
		Serial.println("- compass initialized");
	} else {
		Serial.println("! compass init failed!");
	}
	// IMU
	Serial.print("Initializing imu:");
	if (imu_init()) {
		Serial.println("- imu initialized");
	} else {
		Serial.println("! imu init failed!");
	}
		*/
#if RIVERMOTE
	// Spectral
	Serial.print("Initializing spectral:");
	if (spectral_init()) {
		Serial.println("- spectral initialized");
	} else {
		Serial.println("! spectral init failed!");
	}
#endif // RIVERMOTE
#if MINIMOTE
	// Particulate matter
	Serial.print("Initializing particulate matter sensor:");
	if (pm_init()) {
		Serial.println("- pm sensor initialized");
	} else {
		Serial.println("! pm sensor init failed!");
	}
	// Enviromental sensors
	if (env_init()) {
		Serial.println("- env sensorsinitialized");
	} else {
		Serial.println("! env sensors init failed!");
	}
	// Ozone sensor
	if (ozone_init){
		Serial.println("- ozone sensor initialized");
	} else {
		Serial.println("! ozone sensor init failed!");
	}
	/*
	// Air speed and velocity
	if (velo_init){
		Serial.println("- air velocity initialized");
	} else {
		Serial.println("! air velocity init failed!");
	}
	*/
	
#endif // MINIMOTE

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
	// Control system
	control_init();

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
#if RIVERMOTE
	flash_beacon(); // beacon light
	// Motor control
	control_handle_input(bluetooth_get_pressed());
	control_autonomous_mode();

	// If enough time has passed from last log...
	static unsigned long lastLog = 0;
	if (millis() - lastLog > LOG_INTERVAL) {
		// Read sensors
		String time = modem_gps_read_time();
		ModemGPSData gps = modem_gps_read();
		float temperature = temp_read();
		float turbidity = get_turbidity();
		float tds = get_tds();
		SpectralData spectral = get_spectrum();
		// Log to sd and bluetooth
		sd_appendf("%lu,%s,%d,%.3f,%.6f,%.6f,%.2f,%.2f,%.2f,%.2f,%.4f,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", 
			millis(), time.c_str(), pmu_get_battery_percent(), pmu_get_battery_voltage(),
			gps.lat, gps.lng, gps.speed, gps.track, compass_read(),
			temperature, turbidity, tds,
			spectral.violet, spectral.blue, spectral.green, spectral.yellow, spectral.orange, spectral.red);
		bluetooth_printf("%.3fV %d%% %.0f%%, %s %s\n",
						 pmu_get_battery_voltage(), pmu_get_battery_percent(),
						 motors_get_max_speed() * 100, control_is_autonomous() ? "A" : "M", modem_gps_fixed() ? "FIX" : "NO FIX");
		lastLog = millis();
	}
#endif

#if MINIMOTE
	float temperature = temp_read();
	float turbidity = get_turbidity();
	float tds = get_tds(); Serial.println(tds);
	//EnvData e_data = env_read(); //Serial.println(e_data.tempF);
	//PMData p_data = pm_read(); //Serial.println(p_data.pm2_5);
	//float air_velo = velo_read(); Serial.println(air_velo); //-- BROKEN IN LIBRARY --//
	//double ozone = ozone_read(); Serial.println(ozone);
	
	delay(1000);

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
