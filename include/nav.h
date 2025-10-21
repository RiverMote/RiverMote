#pragma once

/**
 * Calculates the bearing between two points.
 * @param latA latitude of the first point
 * @param lngA longitude of the first point
 * @param latB latitude of the second point
 * @param lngB longitude of the second point
 * @return bearing in degrees
 */
double calculate_bearing(double latA, double lngA, double latB, double lngB);

/**
 * Calculates the distance between two points.
 * @param latA latitude of the first point
 * @param lngA longitude of the first point
 * @param latB latitude of the second point
 * @param lngB longitude of the second point
 * @return distance in meters
 */
double calculate_distance(double latA, double lngA, double latB, double lngB);
