#pragma once

#include "esphome.h"

namespace esphome {
namespace latching_relay {

class LatchingRelay : public esphome::Component, public esphome::switch_::Switch {

public:
    LatchingRelay(int pin_set, int pin_reset, bool assumed_state);

    bool assumed_state() override;
    void setup() override;
    void write_state(bool state) override;
    void set();
    void reset();

private:
    int state;
    int pin_set;
    int pin_reset;
    bool is_assumed_state;
};

}
}
