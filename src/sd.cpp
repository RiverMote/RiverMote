#include <FS.h>
#include <stdarg.h>
#include <SD_MMC.h>

#include "pins.h"

#include "sd.h"

#define FORMAT_BUFFER_SIZE 512 // Buffer size for sd_appendf

static File dataFile;
static bool sdReady = false; // True if initialized and log file open

/**
 * Generate the next available filename in the format /data_XX.csv
 * where XX is a zero-padded number starting from 01.
 * @return next available filename as a String
 */
static String sd_next_filename() {
    int idx = 1;
    char fname[20];
    while (true) {
        snprintf(fname, sizeof(fname), "/data_%04d.csv", idx);
        if (!SD_MMC.exists(fname)) {
            return String(fname);
        }
        idx++;
        if (idx > 9999) break; // Limit to 9999 files
    }
    return String("/data.csv"); // Fallback
}

bool sd_init(bool format_on_fail) {
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    return SD_MMC.begin("/sdcard", true, format_on_fail);
}

bool sd_create_new(const char *header) {
    String filename = sd_next_filename();
    dataFile = SD_MMC.open(filename, FILE_WRITE);
    if (!dataFile) {
        return false;
    }
    Serial.printf("logging to %s\n", filename.c_str());

    // Write header if applicable
    if (header) {
        dataFile.println(header);
        dataFile.flush();
    }
    sdReady = true;
    return true;
}

bool sd_is_ready() {
    return sdReady;
}

bool sd_append(const char *line) {
    if (!sdReady || !dataFile) {
        return false;
    }
    if (dataFile.println(line)) {
        dataFile.flush();
        return true;
    }
    return false;
}

bool sd_appendf(const char *fmt, ...) {
    char buf[FORMAT_BUFFER_SIZE];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return sd_append(buf);
}
