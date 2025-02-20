#pragma once

#include "esphome.h"

namespace esphome {
namespace switch_tree {

class SwitchTree : public esphome::Component, public esphome::number::Number {
public:
    SwitchTree(
        esphome::switch_::Switch* s1,
        esphome::switch_::Switch* s2,
        esphome::switch_::Switch* s3,
        esphome::switch_::Switch* s4,
        esphome::switch_::Switch* s5,
        esphome::switch_::Switch* s6,
        esphome::switch_::Switch* s7
    );

    void setup() override;

    void control(float value);
    bool set_port(int port);

    void set_restore_value(bool restore_value) { this->restore_value = restore_value; }

protected:
    esphome::switch_::Switch* s1;
    esphome::switch_::Switch* s2;
    esphome::switch_::Switch* s3;
    esphome::switch_::Switch* s4;
    esphome::switch_::Switch* s5;
    esphome::switch_::Switch* s6;
    esphome::switch_::Switch* s7;

    bool restore_value;
    esphome::ESPPreferenceObject pref;
};

}
}
