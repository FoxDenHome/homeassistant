#include "esphome.h"

#define RELAY_KEEP_ON_TIME 10

#define STATE_UNKNOWN 0
#define STATE_SET 1
#define STATE_RESET 2

class LatchingRelayComponent : public Component, public Switch {
    public:
    LatchingRelayComponent(int pin_set, int pin_reset, bool assumed_state) {
        this->pin_set = pin_set;
        this->pin_reset = pin_reset;
        this->is_assumed_state = assumed_state;
        this->state = STATE_UNKNOWN;
    }

    LatchingRelayComponent(int pin_set, int pin_reset) : LatchingRelayComponent(pin_set, pin_reset, true) {
    }

    bool assumed_state() override {
        return this->is_assumed_state;
    }

    void setup() override {
        pinMode(this->pin_set, OUTPUT);
        pinMode(this->pin_reset, OUTPUT);
        digitalWrite(this->pin_set, LOW);
        digitalWrite(this->pin_reset, LOW);
    }

    void write_state(bool state) override {
        if (state) {
            this->set();
        } else {
            this->reset();
        }
        publish_state(state);
    }

    void set() {
        if (this->state == STATE_SET) {
            return;
        }

        this->state = STATE_SET;
        digitalWrite(this->pin_set, HIGH);
        delay(RELAY_KEEP_ON_TIME);
        digitalWrite(this->pin_set, LOW);
    }

    void reset() {
        if (this->state == STATE_RESET) {
            return;
        }

        this->state = STATE_RESET;
        digitalWrite(this->pin_reset, HIGH);
        delay(RELAY_KEEP_ON_TIME);
        digitalWrite(this->pin_reset, LOW);
    }

    private:
    int state;
    int pin_set;
    int pin_reset;
    bool is_assumed_state;
};
