#pragma once

/**
 * Initialize the magnetic compass.
 * @return true on successful initialization
 */
bool compass_init();

/**
 * @return current magnetic compass angle in degrees
 */
float compass_read();
