#include "esphome.h"

#define PACKET_THRESHOLD_MS 5
#define PACKET_LENGTH 4
#define HEIGHT_MIN 500
#define HEIGHT_MAX 1500
#define HEIGHT_THROTTLE 100

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
            this->serialBuffer[this->serialBufferPos] = read();
            //ESP_LOGI("UPLIFT", "Got %x at %d\n", this->serialBuffer[this->serialBufferPos], this->serialBufferPos);
            if (++this->serialBufferPos >= PACKET_LENGTH) {
                this->handlePacket();
                this->serialBufferPos = 0;
            }
        }
    }

    protected:
    unsigned long lastSerialData;
    int serialBuffer[PACKET_LENGTH];
    int serialBufferPos = 0;
    uint16_t lastHeight = 0;
    unsigned long lastHeightUpdate;

    void handlePacket() {
        if (this->serialBuffer[0] != 0x01) {
            return;
        }
        const unsigned long now = millis();
        switch (this->serialBuffer[1]) {
            case 0x01: { // display height.
                const uint16_t height = ((uint16_t)this->serialBuffer[2] << 8) | (uint16_t)this->serialBuffer[3];
                if (height != lastHeight && height >= HEIGHT_MIN && height <= HEIGHT_MAX && now - lastHeightUpdate >= HEIGHT_THROTTLE) {
                    lastHeight = height;
                    lastHeightUpdate = now;
                    id(desk_height).publish_state(height);
                }
                break;
            }
            case 0x02: {// error
                break;
            }
            case 0x06: {// S-X / save preset
                break;
            }
        }
    }
};
