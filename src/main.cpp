#include <Arduino.h>
#include <Wire.h>

#include "sensors/imu.h"
#include "sensors/temp.h"
#include "sensors/turbidity.h"
#include "sensors/tds.h"
#include "modem.h"
#include "pins.h"
#include "pmu.h"
#include "sd.h"
#include "flasher.h"

#define WAIT_FOR_SERIAL 0
#define WAIT_FOR_GPS_FIX 0

#define BUFF_SIZE 100

double avgBuffer[BUFF_SIZE];

double getAverage(double new_val) {
    // add new value to buffer
    //buffer.pop();	
    for(int i=BUFF_SIZE-1; i >= 1; i--) {
        // shuffle everything right by 1
        double curr_val = avgBuffer[i-1]; // move everything one to the left
        avgBuffer[i] =  curr_val;
    }
    avgBuffer[0] = new_val; // add new val to the beginning of the array
	
    // get average 
	double avg = 0;
    double sum = 0;
    for(int i=0; i < BUFF_SIZE; i++){
		//val = avgBuffer[i];
        sum = sum + avgBuffer[i];
        avg = sum/BUFF_SIZE;
    }
    return avg; 
}

void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL
	while (!Serial);
	delay(1000);
#endif
	Serial.println("Welcome to river mote!");

	// Initialize PMU
	Serial.println("Initializing pmu:");
    if (!pmu_init()) {
		Serial.println("! pmu init failed! halting.");
		while (true) yield();
	}
	Serial.println("- pmu initialized");
	
	// Initialize modem
	Serial.println("Initializing modem:");
	if (modem_init()) {
		Serial.println("- modem initialized");
	} else {
		Serial.println("! modem init failed!");
	}

	// Flasher
	flasher_init();

	// Enable GPS and wait for fix
	Serial.println("enabling modem gps");
	if (modem_gps_enable()) {
#if WAIT_FOR_GPS_FIX
		Serial.print("modem gps enabled, waiting for fix");
		while (!modem_gps_fixed()) {
			Serial.print(".");
			delay(1000);
		}
#else
		Serial.println("modem gps enabled");
#endif
	} else {
		Serial.println("modem gps enable failed!");
	}

	// Initialize sensors
	Serial.println("Initializing i2c bus:");
	Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ);
	// Temperature sensor
	Serial.println("Initializing temperature sensor:");
	if (temp_init()) {
		Serial.println("- temperature sensor initialized");
	} else {
		Serial.println("! temperature sensor init failed!");
	}
	// IMU
	Serial.println("Initializing imu:");
	if (imu_init()) {
		Serial.println("- imu initialized");
	} else {
		Serial.println("! imu init failed!");
	}

	// Initialize SD card
	Serial.println("Initializing sd card:");
	if (sd_init()) {
		Serial.println("- sd initialized");
	} else {
		Serial.println("! sd init failed!");
	}

	Serial.println("Ready!");
}

void loop() {
	flash_beacon(); // beacon light
	//long val[] = imu_read(); Serial.println(val(0));
	//#define TEST_COORD 41.906334290146226 // 41.906334
	//trim_gps(TEST_COORD);
}
