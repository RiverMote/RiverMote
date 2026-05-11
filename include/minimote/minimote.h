#pragma once

#if MINIMOTE

#include <ArduinoJson.h>

void minimote_init(JsonDocument &initted);
void minimote_tick();

#endif
