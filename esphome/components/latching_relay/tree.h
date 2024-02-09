#include "esphome.h"

static void switch_write_state(Switch* sw, bool state) {
    if (state) {
        sw->turn_on();
    } else {
        sw->turn_off();
    }
}

void write_relay_tree_state(int port) {
    if (port < 1 || port > 8) {
        return;
    }

    port -= 1;

    const bool set_cur_1 = port > 3;
    const bool set_cur_2 = (port % 4) > 1;
    const bool set_cur_3 = (port % 2) > 0;

    if (set_cur_1) {
        if (set_cur_2) {
            switch_write_state(&id(k7), set_cur_3); // A = 7, B = 8
        } else {
            switch_write_state(&id(k6), set_cur_3); // A = 5, B = 6
        }
        switch_write_state(&id(k3), set_cur_2); // A = 56, B = 78
    } else {
        if (set_cur_2) {
            switch_write_state(&id(k5), set_cur_3); // A = 3, B = 4
        } else {
            switch_write_state(&id(k4), set_cur_3); // A = 1, B = 2
        }
        switch_write_state(&id(k2), set_cur_2); // A = 12, B = 34
    }
    switch_write_state(&id(k1), set_cur_1); // A = 1234, B = 5678
}
