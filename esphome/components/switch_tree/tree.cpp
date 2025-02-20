#include "tree.h"
#include "esphome.h"

#define INITIAL_VALUE 0

namespace esphome {
namespace switch_tree {

static void switch_write_state(esphome::switch_::Switch* sw, bool state) {
    if (state) {
        sw->turn_on();
    } else {
        sw->turn_off();
    }
}

SwitchTree::SwitchTree(
    esphome::switch_::Switch* s1,
    esphome::switch_::Switch* s2,
    esphome::switch_::Switch* s3,
    esphome::switch_::Switch* s4,
    esphome::switch_::Switch* s5,
    esphome::switch_::Switch* s6,
    esphome::switch_::Switch* s7
) {
    this->s1 = s1;
    this->s2 = s2;
    this->s3 = s3;
    this->s4 = s4;
    this->s5 = s5;
    this->s6 = s6;
    this->s7 = s7;
}

void SwitchTree::setup() {
    float value;
    if (!this->restore_value) {
        value = INITIAL_VALUE;
    } else {
        this->pref = global_preferences->make_preference<float>(this->get_object_id_hash());
        if (!this->pref.load(&value)) {
            value = INITIAL_VALUE;
        }
    }
    this->publish_state(value);
}

void SwitchTree::control(float value) {
    if (this->set_port((int)value)) {
        this->publish_state(value);
        if (this->restore_value) {
            this->pref.save(&value);
        }
    }
}

bool SwitchTree::set_port(int port) {
    if (port < 1 || port > 8) {
        return false;
    }

    port -= 1;

    const bool set_cur_1 = port > 3;
    const bool set_cur_2 = (port % 4) > 1;
    const bool set_cur_3 = (port % 2) > 0;

    if (set_cur_1) {
        if (set_cur_2) {
            switch_write_state(this->s7, set_cur_3); // A = 7, B = 8
        } else {
            switch_write_state(this->s6, set_cur_3); // A = 5, B = 6
        }
        switch_write_state(this->s3, set_cur_2); // A = 56, B = 78
    } else {
        if (set_cur_2) {
            switch_write_state(this->s5, set_cur_3); // A = 3, B = 4
        } else {
            switch_write_state(this->s4, set_cur_3); // A = 1, B = 2
        }
        switch_write_state(this->s2, set_cur_2); // A = 12, B = 34
    }
    switch_write_state(this->s1, set_cur_1); // A = 1234, B = 5678

    return true;
}

}
}

