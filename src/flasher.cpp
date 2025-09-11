/* a function to check light level
* and flash beacon LED if dark
*/

#include <Arduino.h>
#include "pins.h"

const int blinkInterval = 5000;
const int blinkDuration = 75;
long lastBlink = 0;
int blinkState = 0;

const int lightThreshold = 2500; // below this for dark

void flasher_init() {
    pinMode(PIN_CDS, INPUT);
    pinMode(PIN_FLASHER, OUTPUT);
}

void flash_beacon() {
    int lightLevel = analogRead(PIN_CDS); // get the light level
        
    if(lightLevel < lightThreshold) { 
        if(millis()%blinkInterval == 0 && blinkState == 0) {
            blinkState = 1;
            digitalWrite(PIN_FLASHER, blinkState);
            lastBlink = millis();
            //Serial.print("Light level: "); Serial.println(lightLevel);
        } 
        if(millis() - lastBlink > blinkDuration) {
            blinkState = 0;
            digitalWrite(PIN_FLASHER, blinkState);
            //Serial.print("Light level: "); Serial.println(lightLevel);
        }     
        
    } else {
        digitalWrite(PIN_FLASHER, 0);
    }
}
    

