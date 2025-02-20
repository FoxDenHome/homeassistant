#pragma once

#include "esphome.h"

namespace esphome {
namespace uplift {

class Uplift : public esphome::sensor::Sensor, public esphome::Component, public esphome::uart::UARTDevice {

public:
    Uplift() : esphome::uart::UARTDevice() {}

    void setup() override;
    void loop() override;

protected:
    unsigned long lastSerialData = 0;
    uint8_t serialBuffer[4]; // PACKET_LENGTH
    int serialBufferPos = 0;
    uint16_t lastHeight = 0;
    unsigned long lastHeightUpdate = 0;

    uint16_t lastErrorCode = 0;
    uint16_t lastResetCode = 0;
    uint8_t lastSavePreset = 0xFE;
    uint8_t lastCommand = 0;

    void handlePacket();

};

}
}
