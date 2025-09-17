#include <Arduino.h>
#include "pins.h"
#include "pmu.h"
#include "debugging.h"
#include "sd.h"

// TESTING //
//long loopMillis;
//double reportMinutes = 0.25; 

String twoDigit(int number){
    char strNum[2];
    if(number < 10) {
        sprintf(strNum, "0%d", number);
    } else {
        sprintf(strNum, "%d", number);
    }
    return strNum;
}

String report_uptime() {
    char tString[12];
    long mSeconds = millis()/1000.0; 
    int s = mSeconds%60;
    String strS = twoDigit(s);
    //int m = mSeconds%360;
    int m = mSeconds/60;
    String strM = twoDigit(m);
    int h = mSeconds/360;
    sprintf(tString, "%d:%s:%s" , h,strM,strS);
    return tString;
}


void debug_reporter() {	
    char data[200];

	sprintf(data, "%s battV=%d", report_uptime(), pmu_get_battery_voltage());
    Serial.println(data);
    sd_append(data);
}