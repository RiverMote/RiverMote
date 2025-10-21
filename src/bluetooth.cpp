#include "bluetooth.h"

#if RIVERMOTE

#include <NimBLEDevice.h>
#include <stdarg.h>

// Nordic UART Service (NUS) UUIDs used by Bluefruit app
#define UUID_NUS_SERVICE "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UUID_NUS_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UUID_NUS_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define FORMAT_BUFFER_SIZE 512 // Buffer size for bluetooth_printf

static NimBLEServer *server = nullptr;
static NimBLECharacteristic *serverRx = nullptr, *serverTx = nullptr;
static uint8_t pressedMask = 0;

/**
 * Parse a received packet from the Control Pad and update the pressed bitmask.
 * @param packet received packet string
 */
static void parse_packet(const String &packet) {
    /**
     * Adafruit Bluefruit Control Pad sends ASCII packets like so:
     * "!Bx1c" = button x pressed
     * "!Bx0c" = button x released
     * where,
     * 5 - up, 6 - down, 7 - left, 8 - right
     * and c is a CRC checksum (not checked here)
     */

    // Check that we have a valid button packet
    if (packet.length() < 5 || packet.substring(0, 2) != "!B") {
        return;
    }
    // Parse the button number and state
    int btn = packet.charAt(2) - '0'; // Number of button 1-8
    bool pressed = packet.charAt(3) == '1'; // True if pressed, false if released
    int bit = 7 - (btn - 1); // Convert numbered button to bit index 7..0 
    // Set the corresponding bit in the pressed bitmask
    uint8_t mask = (1u << bit);
    if (pressed) {
        pressedMask |= mask;
    } else {
        pressedMask &= ~mask;
    }
}

// Custom callback class to handle when data is received
class RxCallbacks : public NimBLECharacteristicCallbacks {
    // Parses (and optionally logs) received data
	void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override {
		String packet = c->getValue();
#if LOG_BLUETOOTH
        Serial.printf("[BT] RX: \"%s\"\n", packet.c_str());
#endif
		if (!packet.isEmpty()) {
            parse_packet(packet);
        }
        (void)info;
	}
};

// Custom callback class to handle when a device connects or disconnects
class ServerCallbacks : public NimBLEServerCallbacks {
	void onConnect(NimBLEServer *s, NimBLEConnInfo &info) override {
		Serial.println("[BT] device connected");
        (void)s;
        (void)info;
	}
	void onDisconnect(NimBLEServer *s, NimBLEConnInfo &info, int reason) override {
        Serial.println("[BT] device disconnected");
		NimBLEDevice::startAdvertising();
        (void)s;
        (void)info;
        (void)reason;
	}
};

bool bluetooth_init() {
    // Initialize NimBLE
	NimBLEDevice::init("RiverMote");
	NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Max TX power for max range
    // Prefer LE Coded (S=2/S=8) for longer range if supported by both peers
    NimBLEDevice::setDefaultPhy(BLE_GAP_LE_PHY_CODED_MASK, BLE_GAP_LE_PHY_CODED_MASK);

    // Create server
	server = NimBLEDevice::createServer();
	server->setCallbacks(new ServerCallbacks());
    // Create NUS service and start it
	NimBLEService *service = server->createService(UUID_NUS_SERVICE);
	serverTx = service->createCharacteristic(UUID_NUS_TX, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
	serverRx = service->createCharacteristic(UUID_NUS_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
	serverRx->setCallbacks(new RxCallbacks());
	if (!service->start()) {
        Serial.println("failed to start NUS service");
        return false;
    }

    // Start advertising
    NimBLEDevice::setDeviceName("RiverMote");
	NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->enableScanResponse(true);
	adv->addServiceUUID(UUID_NUS_SERVICE);
	if (!adv->start()) {
        Serial.println("failed to start advertising");
        return false;
    }
	return true;
}

uint8_t bluetooth_get_pressed() {
	return pressedMask;
}

bool bluetooth_printf(const char *fmt, ...) {
    if (!serverTx || server->getConnectedCount() == 0) {
        return false; // No connection or TX not ready
    }
    // Format the string into a buffer
    char buf[FORMAT_BUFFER_SIZE];
    va_list args; va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    // Send the formatted string as a notification
    if (len > 0 && len < (int)sizeof(buf)) {
        serverTx->setValue((uint8_t*)buf, len);
        return serverTx->notify();
    }
    return false;
}

#endif // RIVERMOTE
