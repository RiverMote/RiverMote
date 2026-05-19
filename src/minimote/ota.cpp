#include "minimote/ota.h"

#if MINIMOTE

#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>
#include <Update.h>

#include "modem.h"

bool ota_do_update(const char *server, const char *path) {
    Serial.printf("starting update from https://%s%s\n", server, path);
    // Create a HTTP client to fetch the firmware from the server
    HttpClient http(modemClient, server, 443);
    // Create a hook to print current OTA progress
    Update.onProgress([](size_t done, size_t total) {
        if (total == 0) {
            return;
        }
        uint8_t percent = (done * 100) / total;
        Serial.printf("ota progress: %u%% (%u/%u)\n", percent, (unsigned)done, (unsigned)total);
    });

    // Make the HTTP request to fetch the firmware
    http.connectionKeepAlive(); // Required for https
    http.get(path);
    int statusCode = http.responseStatusCode();
    if (statusCode != 200) {
        Serial.printf("update failed, HTTP status code: %d\n", statusCode);
        return false;
    }

    // Stream the firmware from the server and write it to flash
    size_t contentLength = http.contentLength();
    if (!Update.begin(contentLength)) {
        Serial.printf("update failed, could not begin update: %s\n", Update.errorString());
        return false;
    }
    size_t written = Update.writeStream(http);
    if (written != contentLength) {
        Serial.printf("update failed, written %d of %d bytes\n", written, contentLength);
        Update.abort();
        return false;
    }
    if (!Update.end()) {
        Serial.printf("update failed, error: %s\n", Update.errorString());
        return false;
    }
    return true;
}

#endif // MINIMOTE