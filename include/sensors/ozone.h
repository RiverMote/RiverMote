#pragma once

bool ozone_init();

/* read ozone (0-10ppm)
original lib deifnes value as int_16t */

double ozone_read();