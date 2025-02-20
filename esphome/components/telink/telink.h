#pragma once

#include "esphome.h"
#include <string.h>

namespace esphome {
namespace telink {

class Telink : public esphome::Component, public esphome::binary_sensor::BinarySensor {
public:
    Telink(esphome::ble_client::BLEClient* client, esphome::script::Script<std::vector<uint8_t>>* command_script, const char* mac, const char* name);

    float handle_notify(std::vector<uint8_t> payload);
    float handle_pairing_data(std::vector<uint8_t> payload);

    void shutdown();

    std::vector<uint8_t> make_pairing_data();

    void set_light();
    bool get_is_paired() { return this->is_paired; }

    uint8_t light_red;
    uint8_t light_green;
    uint8_t light_blue;
    uint8_t light_motor;
    uint8_t light_laser;
    uint8_t light_brightness;
    bool light_breathe;

protected:
    void send_command(const uint8_t command, const uint8_t* payload, const int payload_len);
    void make_ivm(uint8_t* ivm, const uint32_t sequence_number);
    void make_ivs(uint8_t* ivm, uint8_t* payload);

    uint8_t session_key[16];
    uint8_t mesh_password[16];
    uint8_t name[16];
    uint8_t mac[6];
    uint8_t reversed_mac[6];
    uint32_t sequence_number;
    bool is_paired;

    uint8_t login_random[8];
    uint16_t mesh_address;
    uint16_t vendor_id;
    esphome::ble_client::BLEClient* client;
    esphome::script::Script<>* notify_script;
    esphome::script::Script<std::vector<uint8_t>>* command_script;
};

}
}
