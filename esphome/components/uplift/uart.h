#include "esphome.h"

#define PACKET_THRESHOLD_MS 5
#define PACKET_LENGTH 4
#define HEIGHT_MIN 500
#define HEIGHT_MAX 1500
#define HEIGHT_THROTTLE 100

#define DISP_HEIGHT 0x01
#define DISP_ERROR 0x02
#define DISP_RESET 0x04
#define DISP_SAVE 0x06

class UpliftDeskSerialComponent : public Component, public UARTDevice {
    public:
    UpliftDeskSerialComponent(UARTComponent *parent) : UARTDevice(parent) {}

    void setup() override {
        this->lastSerialData = millis();
        this->lastHeightUpdate = millis();
        this->serialBufferPos = 0;
        memset(this->serialBuffer, 0, PACKET_LENGTH * sizeof(int));
    }
    void loop() override {
        const unsigned long now = millis();
        while (available()) {
            if (now - this->lastSerialData > PACKET_THRESHOLD_MS) {
                this->serialBufferPos = 0;
            }
            this->lastSerialData = now;
            this->serialBuffer[this->serialBufferPos] = (uint8_t)read();
            if (++this->serialBufferPos >= PACKET_LENGTH) {
                this->handlePacket();
                this->serialBufferPos = 0;
            }
        }
    }

    protected:
    unsigned long lastSerialData;
    uint8_t serialBuffer[PACKET_LENGTH];
    int serialBufferPos = 0;
    uint16_t lastHeight = 0;
    unsigned long lastHeightUpdate;

    uint16_t lastErrorCode = 0;
    uint16_t lastResetCode = 0;
    uint8_t lastSavePreset = 0xFE;
    uint8_t lastCommand = 0;

    void handlePacket() {
        if (this->serialBuffer[0] != 0x01) {
            return;
        }

        const uint8_t command = this->serialBuffer[1];
        const bool commandChanged = command != this->lastCommand;
        this->lastCommand = command;

        const unsigned long now = millis();
        switch (command) {
            case DISP_HEIGHT: { // display height.
                const uint16_t height = ((uint16_t)this->serialBuffer[2] << 8) | (uint16_t)this->serialBuffer[3];
                if (height != lastHeight && height >= HEIGHT_MIN && height <= HEIGHT_MAX && now - lastHeightUpdate >= HEIGHT_THROTTLE) {
                    lastHeight = height;
                    lastHeightUpdate = now;
                    id(desk_height).publish_state(height);
                }
                break;
            }
            case DISP_ERROR: { // error
                const uint16_t errorCode = ((uint16_t)this->serialBuffer[2] << 8) | (uint16_t)this->serialBuffer[3];
                if (errorCode != this->lastErrorCode || commandChanged) {
                    ESP_LOGW("UPLIFT", "Error code %x", errorCode);
                    this->lastErrorCode = errorCode;
                }
                break;
            }
            case DISP_RESET: { // reset
                const uint16_t resetCode = ((uint16_t)this->serialBuffer[2] << 8) | (uint16_t)this->serialBuffer[3];
                if (resetCode != this->lastResetCode || commandChanged) {
                    ESP_LOGW("UPLIFT", "Reset code %x", resetCode);
                    this->lastResetCode = resetCode;
                }
                break;
            }
            case DISP_SAVE: { // S-X / save preset
                if (this->serialBuffer[3] != 0) {
                    return;
                }

                uint8_t preset = 0xFF;
                const uint8_t rawPreset = this->serialBuffer[2];
                switch (rawPreset) {
                    case 0b0000:
                        preset = 0;
                        break;
                    case 0b0001:
                        preset = 1;
                        break;
                    case 0b0010:
                        preset = 2;
                        break;
                    case 0b0100:
                        preset = 3;
                        break;
                    case 0b1000:
                        preset = 4;
                        break;
                }

                if (preset != this->lastSavePreset) {
                    ESP_LOGI("UPLIFT", "Save preset %d", preset);
                    this->lastSavePreset = preset;
                }
                break;
            }
        }
    }
};
