#include "esphome.h"
#include <string.h>

#include "telink.h"
#include "tinyaes.h"

using esphome::esp_log_printf_;

#define BLE_GATT_OP_PAIR_ENC_REQ 0x0C
#define BLE_GATT_OP_PAIR_ENC_RSP 0x0D
#define BLE_GATT_OP_PAIR_ENC_FAIL 0x0E

#define COMMAND_FIND_MESH 0xE4
#define COMMAND_LIGHT_CONTROL 0xF0
#define MESH_ADDRESS_BROADCAST 0xFFFF

#define LIGHT_SET_DIRECT 71

#define PLAIN_HEADER_LEN_COMMAND 3
#define PLAIN_HEADER_LEN_NOTIFY 5

static void bytes_xor(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, const int len) {
    for (int i = 0; i < len; i++) {
        dst[i] = src1[i] ^ src2[i];
    }
}

#define bytes_xor_16(dst, src1, src2) bytes_xor(dst, src1, src2, 16)

static void reverse_len(uint8_t* dst, const uint8_t* src, const int len) {
    const int imax = len - 1;
    for (int i = 0; i <= imax; i++) {
        dst[imax - i] = src[i];
    }
}

#define reverse_16(dst, src) reverse_len(dst, src, 16)

static void telink_aes_base_encrypt(uint8_t* dst, const uint8_t *key, const uint8_t *data) {
    struct AES_ctx aes;

    uint8_t key_reversed[16];
    reverse_16(key_reversed, key);
    AES_init_ctx(&aes, key_reversed);

    reverse_16(dst, data);
    AES_ECB_encrypt(&aes, dst);
}

static void telink_aes_att_encrypt(uint8_t* dst, const uint8_t *key, const uint8_t *data) {
    uint8_t tmp[16];
    telink_aes_base_encrypt(tmp, key, data);
    reverse_16(dst, tmp);
}

static void telink_aes_ivm_encrypt(uint8_t* dst, const uint8_t *key, const uint8_t* ivm, const uint8_t *payload, const int payload_len, const int plain_header_len) {
    memcpy(dst, payload, payload_len);

    const int offset_after_check = plain_header_len + 2;
    const int encrypted_len = payload_len - offset_after_check;

    // Stage 1
    uint8_t ivm_padded[16];
    memset(ivm_padded, 0, 16);
    memcpy(ivm_padded, ivm, 8);
    ivm_padded[8] = encrypted_len;

    uint8_t encrypted_1[16];
    telink_aes_att_encrypt(encrypted_1, key, ivm_padded);
    for (int i = 0; i < encrypted_len; i++) {
        encrypted_1[i] ^= payload[i + offset_after_check];
    }
    uint8_t encrypted_2[16];
    telink_aes_att_encrypt(encrypted_2, key, encrypted_1);
    dst[plain_header_len] = encrypted_2[0];
    dst[plain_header_len + 1] = encrypted_2[1];

    // Stage 2
    ivm_padded[0] = 0;
    memcpy(ivm_padded + 1, ivm, 8);
    telink_aes_att_encrypt(encrypted_1, key, ivm_padded);
    for (int i = 0; i < encrypted_len; i++) {
        dst[i + offset_after_check] ^= encrypted_1[i];
    }
}

static int telink_aes_ivm_decrypt(uint8_t* dst, const uint8_t *key, const uint8_t* ivm, const uint8_t *payload, const int payload_len, const int plain_header_len) {
    memcpy(dst, payload, payload_len);

    const int offset_after_check = plain_header_len + 2;
    const int encrypted_len = payload_len - offset_after_check;

    // Stage 2
    uint8_t ivm_padded[16];
    memset(ivm_padded, 0, 16);
    memcpy(ivm_padded + 1, ivm, 8);

    uint8_t encrypted_1[16];
    telink_aes_att_encrypt(encrypted_1, key, ivm_padded);
    for (int i = 0; i < encrypted_len; i++) {
        dst[i + offset_after_check] ^= encrypted_1[i];
    }

    // Stage 1
    memcpy(ivm_padded, ivm, 8);
    ivm_padded[8] = encrypted_len;

    telink_aes_att_encrypt(encrypted_1, key, ivm_padded);
    for (int i = 0; i < encrypted_len; i++) {
        encrypted_1[i] ^= dst[i + offset_after_check];
    }
    uint8_t encrypted_2[16];
    telink_aes_att_encrypt(encrypted_2, key, encrypted_1);

    if (memcmp(dst + plain_header_len, encrypted_2, 2)) {
        ESP_LOGW("telink_decrypt", "Encryption check mismatch");
        return -1;
    }
    return 0;
}

#define COMMAND_BASE_LEN (3 + 2 + 2 + 1 + 2)
#define COMMAND_MAX_PAYLOAD 10
#define COMMAND_FULL_LEN (COMMAND_BASE_LEN + COMMAND_MAX_PAYLOAD)

namespace esphome {
namespace telink {

Telink::Telink(esphome::ble_client::BLEClient* client, esphome::script::Script<std::vector<uint8_t>>* command_script, const char* mac, const char* name) {
    this->command_script = command_script;
    memcpy(this->mac, mac, 6);
    reverse_len(this->reversed_mac, this->mac, 6);

    memset(this->mesh_password, 0, 16);
    memcpy(this->mesh_password, "123\0", 4);

    memset(this->name, 0, 16);
    strcpy((char*)this->name, name);

    this->client = client;
    this->sequence_number = 1337;
    for (int i = 0; i < 8; i++) {
        this->login_random[i] = i;
    }

    this->light_red = 0xFF;
    this->light_green = 0xFF;
    this->light_blue = 0xFF;
    this->light_laser = 0xFF;
    this->light_motor = 0xFF;
    this->light_brightness = 0xFF;
    this->light_breathe = false;
    this->is_paired = false;
    this->publish_state(false);
}

float Telink::handle_notify(std::vector<uint8_t> payload_vec) {
    if (!this->is_paired) {
        ESP_LOGW("telink_notify", "Got notify when not paired!");
        return -1.0;
    }

    int payload_len = payload_vec.size();
    if (payload_len > 32) {
        ESP_LOGW("telink_notify", "Got notify longer than 32 bytes!");
        return -2.0;
    }

    uint8_t payload[32];
    for (int i = 0; i < payload_len; i++) {
        payload[i] = payload_vec[i];
    }

    uint8_t ivm[8];
    this->make_ivs(ivm, payload);

    uint8_t dst[32];
    if (telink_aes_ivm_decrypt(dst, this->session_key, ivm, payload, payload_len, PLAIN_HEADER_LEN_NOTIFY)) {
        ESP_LOGW("telink_notify", "Got notify decrypt error!");
        return -3.0;
    }

    const uint16_t old_mesh_address = this->mesh_address;
    this->vendor_id = dst[8] | (dst[9] << 8);
    this->mesh_address = dst[3] | (dst[4] << 8);

    if (old_mesh_address != this->mesh_address) {
        ESP_LOGI("telink_pairing", "New mesh found!");
        this->set_light();
    }

    return 1.0;
}

float Telink::handle_pairing_data(std::vector<uint8_t> payload) {
    if (payload[0] != BLE_GATT_OP_PAIR_ENC_RSP) {
        ESP_LOGW("telink_pairing", "invalid pairing response");
        return -1.0;
    }

    uint8_t device_random[16];
    uint8_t device_check[8];
    memset(device_random, 0, 16);
    for (int i = 0; i < 8; i++) {
        device_random[i] = payload[i + 1];
        device_check[i] = payload[i + 1 + 8];
    }

    uint8_t mesh_xor[16];
    bytes_xor_16(mesh_xor, this->name, this->mesh_password);

    uint8_t encrypt_check[16];
    telink_aes_att_encrypt(encrypt_check, device_random, mesh_xor);
    if (memcmp(encrypt_check, device_check, 8)) {
        ESP_LOGW("telink_pairing", "invalid pairing encrypt_check");
        return -2.0;
    }

    uint8_t session_key_base[16];
    memcpy(session_key_base, login_random, 8);
    memcpy(session_key_base + 8, device_random, 8);

    telink_aes_att_encrypt(this->session_key, mesh_xor, session_key_base);

    ESP_LOGI("telink_pairing", "Session key derived OK");

    this->mesh_address = MESH_ADDRESS_BROADCAST;
    this->vendor_id = 0x0211;

    this->is_paired = true;
    this->publish_state(true);
    this->send_command(COMMAND_FIND_MESH, NULL, 0);
    ESP_LOGI("telink_pairing", "Mesh find command sent!");

    return 1.0;
}

void Telink::shutdown() {
    this->is_paired = false;
    this->publish_state(false);
}

std::vector<uint8_t> Telink::make_pairing_data() {
    uint8_t mesh_xor[16];
    bytes_xor_16(mesh_xor, this->name, this->mesh_password);

    uint8_t login_random[16];
    memcpy(login_random, this->login_random, 8);
    memset(login_random + 8, 0, 8);

    uint8_t encrypted_login_packet[16];
    telink_aes_att_encrypt(encrypted_login_packet, login_random, mesh_xor);

    std::vector<uint8_t> full_login_packet;
    full_login_packet.push_back(BLE_GATT_OP_PAIR_ENC_REQ);
    for (int i = 0; i < 8; i++) {
        full_login_packet.push_back(login_random[i]);
    }
    for (int i = 0; i < 8; i++) {
        full_login_packet.push_back(encrypted_login_packet[i]);
    }
    return full_login_packet;
}

void Telink::send_command(const uint8_t command, const uint8_t* payload, const int payload_len) {
    if (!this->is_paired) {
        ESP_LOGW("telink_command", "Got command when not paired!");
        return;
    }

    if (payload_len > COMMAND_MAX_PAYLOAD) {
        ESP_LOGW("telink_command", "payload must be at most 10 bytes");
        return;
    }

    const uint32_t sequence_number = this->sequence_number++;

    uint8_t full_command_packet[COMMAND_FULL_LEN];
    memset(full_command_packet, 0, COMMAND_FULL_LEN);

    full_command_packet[0] = sequence_number & 0xFF;
    full_command_packet[1] = (sequence_number >> 8) & 0xFF;
    full_command_packet[2] = (sequence_number >> 16) & 0xFF;

    //full_command_packet[3] = 0;
    //full_command_packet[4] = 0;

    full_command_packet[5] = this->mesh_address & 0xFF;
    full_command_packet[6] = (this->mesh_address >> 8) & 0xFF;

    full_command_packet[7] = command | 0xC0;

    full_command_packet[8] = this->vendor_id & 0xFF;
    full_command_packet[9] = (this->vendor_id >> 8) & 0xFF;

    memcpy(full_command_packet + COMMAND_BASE_LEN, payload, payload_len);

    uint8_t ivm[8];
    this->make_ivm(ivm, sequence_number);

    uint8_t full_encrypted_packet[COMMAND_FULL_LEN];
    telink_aes_ivm_encrypt(full_encrypted_packet, this->session_key, ivm, full_command_packet, COMMAND_FULL_LEN, PLAIN_HEADER_LEN_COMMAND);

    std::vector<uint8_t> full_encrypted_vector;
    for (int i = 0; i < COMMAND_FULL_LEN; i++) {
        full_encrypted_vector.push_back(full_encrypted_packet[i]);
    }

    this->command_script->execute(full_encrypted_vector);
}

void Telink::set_light() {
    uint8_t set_light_data[8];
    set_light_data[0] = LIGHT_SET_DIRECT;
    set_light_data[1] = this->light_red;
    set_light_data[2] = this->light_green;
    set_light_data[3] = this->light_blue;
    set_light_data[4] = this->light_laser;
    set_light_data[5] = this->light_motor;
    set_light_data[6] = this->light_brightness;
    set_light_data[7] = this->light_breathe ? 1 : 0;

    this->send_command(COMMAND_LIGHT_CONTROL, set_light_data, 8);
}

void Telink::make_ivm(uint8_t* ivm, const uint32_t sequence_number) {
    memcpy(ivm, this->reversed_mac, 4);
    ivm[4] = 1;
    ivm[5] = sequence_number & 0xFF;
    ivm[6] = (sequence_number >> 8) & 0xFF;
    ivm[7] = (sequence_number >> 16) & 0xFF;
}

void Telink::make_ivs(uint8_t* ivm, uint8_t* payload) {
    memcpy(ivm, this->reversed_mac, 3);
    memcpy(ivm + 3, payload, 5);
}

}
}
