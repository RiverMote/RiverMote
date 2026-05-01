/**
 * Source file of pico-fbw: https://github.com/pico-fbw/pico-fbw
 * Licensed under the MIT License
 */

#include "rivermote/nav.h"

#if RIVERMOTE

#include <Arduino.h>
#include <math.h>

#define EARTH_RADIUS_KM 6371                    // Earth's radius in kilometers
#define EARTH_RADIUS_M (EARTH_RADIUS_KM * 1000) // Earth's radius in meters

double calculate_bearing(double latA, double lngA, double latB, double lngB) {
    double thetaA = radians(latA);
    double thetaB = radians(latB);
    double deltaL = radians(lngB) - radians(lngA);

    double y = sin(deltaL) * cos(thetaB);
    double x = cos(thetaA) * sin(thetaB) - sin(thetaA) * cos(thetaB) * cos(deltaL);

    return fmod((degrees(atan2(y, x)) + 360), 360);
}

double calculate_distance(double latA, double lngA, double latB, double lngB) {
    double thetaA = radians(latA);
    double thetaB = radians(latB);
    double deltaT = radians(latB - latA);
    double deltaL = radians(lngB - lngA);

    double a = sin(deltaT / 2) * sin(deltaT / 2) + cos(thetaA) * cos(thetaB) * sin(deltaL / 2) * sin(deltaL / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_M * c;
}

#endif // RIVERMOTE
