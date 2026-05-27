#pragma once

#if MINIMOTE

#include <stddef.h>

/**
 * Take a set of sensor readings and accumulate them towards our averages.
 */
void minimote_sample_accumulate();

/**
 * Get the current sample as a JSON string and write it to the provided buffer.
 * @param payload buffer to write the JSON string to
 * @param payload_length length of the provided buffer
 * @note The sample accumulator will be automatically reset after this function is called.
 */
void minimote_sample_get(char *payload, size_t payload_length);

#endif
