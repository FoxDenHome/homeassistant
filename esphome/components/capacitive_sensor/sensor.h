#include "esphome.h"
#include <CapacitiveSensor.h>

class CapacitiveSensorComponent : public PollingComponent, public Sensor {
    public:
    CapacitiveSensorComponent(uint8_t tx, uint8_t rx, uint16_t samples, char dir = CAPDIR_BOTH) : PollingComponent(100) {
        this->samples = samples;
        this->dir = dir;
        this->sensor = new CapacitiveSensor(tx, rx, dir);
    }

    void setup() override {

    }

    void update() override {
        this->publish_state(this->sensor->capacitiveSensorRaw(this->samples));
    }

    private:
    uint16_t samples;
    char dir;
    CapacitiveSensor *sensor;
};
