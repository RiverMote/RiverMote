/* a function to check light level */

#include <Arduino.h>
#include "pins.h"

void init_cds(){
    pinMode(PIN_CDS, INPUT);
}

//const int lightThreshold = 2500; // below this for dark

int check_cds() {
    int lightLevel = analogRead(PIN_CDS); // get the light level
    return lightLevel;

    // if(lightLevel > lightThreshold) {
    //     return 1;
    // } else {
    //     return 0;
    // }
}