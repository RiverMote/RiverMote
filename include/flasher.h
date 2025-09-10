#pragma once


void flasher_init() {
    pinMode(PIN_CDS, INPUT);
    pinMode(PIN_FLASHER, OUTPUT);
}

/** read light level and flash if dark */
void flash_beacon();