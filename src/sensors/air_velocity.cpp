/*!
 * @file PAV3000_1015.ino
 * @brief This is the example code for the PAV3000_1015 air velocity sensor.
 * @copyright Copyright (c) 2024 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [tangjie](jie.tang@dfrobot.com)
 * @version V1.0
 * @date 2024-12-2
 * @url https://github.com/DFRobot/DFRobot_PAV3000
 */

#include "sensors/air_velocity.h"
#include "DFRobot_PAV3000.h"

DFRobot_PAV3000 fs;

bool velo_init() {
    //if(!fs.setRange(AIRFLOW_RANGE_15_MPS)){
    if(!fs.setRange(AIRFLOW_RANGE_7_MPS)){
        return false;
    } else {
        return true;
    }
}


float velo_read() {
    return fs.readMeterPerSec();
    //return fs.readRaw();
    //return fs.readMilePerHour();
}