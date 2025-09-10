#pragma once

/**
 * Initialize the turbidity sensor.
 */
void tds_init();

/**
 * @return the current turbidity
 */
float get_tds();