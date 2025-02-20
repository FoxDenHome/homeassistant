#include "relay.h"

#define RELAY_KEEP_ON_TIME 10

#define STATE_UNKNOWN 0
#define STATE_SET 1
#define STATE_RESET 2

namespace esphome {
namespace latching_relay {

LatchingRelay::LatchingRelay(int pin_set, int pin_reset, bool assumed_state) {
    this->pin_set = pin_set;
    this->pin_reset = pin_reset;
    this->is_assumed_state = assumed_state;
    this->state = STATE_UNKNOWN;
}

bool LatchingRelay::assumed_state() {
    return this->is_assumed_state;
}

void LatchingRelay::setup() {
    pinMode(this->pin_set, OUTPUT);
    pinMode(this->pin_reset, OUTPUT);
    digitalWrite(this->pin_set, LOW);
    digitalWrite(this->pin_reset, LOW);
}

void LatchingRelay::write_state(bool state) {
    if (state) {
        this->set();
    } else {
        this->reset();
    }
    publish_state(state);
}

void LatchingRelay::set() {
    if (this->state == STATE_SET) {
        return;
    }

    this->state = STATE_SET;
    digitalWrite(this->pin_set, HIGH);
    delay(RELAY_KEEP_ON_TIME);
    digitalWrite(this->pin_set, LOW);
}

void LatchingRelay::reset() {
    if (this->state == STATE_RESET) {
        return;
    }

    this->state = STATE_RESET;
    digitalWrite(this->pin_reset, HIGH);
    delay(RELAY_KEEP_ON_TIME);
    digitalWrite(this->pin_reset, LOW);
}

}
}
