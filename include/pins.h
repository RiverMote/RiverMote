#pragma once

// I2C sensor bus
#define PIN_I2C_SCL 16 // yel
#define PIN_I2C_SDA 17 // blu
#define I2C_FREQ 400000

// OneWire bus for temperature sensor
#define PIN_TEMP 18

// Turbidity 
#define PIN_TURBIDITY 1

// TDS 
#define PIN_TDS 2

// H2O sensors
#define PIN_H2O1 45
#define PIN_H2O2 48
#define PIN_H2O3 47

// Light sensor 
#define PIN_CDS 8 // blu
// Flasher LED
#define PIN_FLASHER 46 // wht (was 21)
/*
// Motor pins
#define PIN_MOTOR1 35
#define PIN_MOTOR2 36
*/

#define PIN_PMU_SDA 15
#define PIN_PMU_SCL 7
#define PIN_PMU_IRQ 6

#define PIN_MODEM_TX 5
#define PIN_MODEM_RX 4
#define PIN_MODEM_RI 3
#define PIN_MODEM_PWR 41
#define PIN_MODEM_DTR 42

#define PIN_SD_CMD 39
#define PIN_SD_CLK 38
#define PIN_SD_D0 40
