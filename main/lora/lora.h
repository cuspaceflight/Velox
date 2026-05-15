#pragma once

#include <driver/spi_master.h>
#include <stdint.h>

#include "lora_defines.h"

typedef struct lora_config_t {
    uint32_t freq;
    int mosi;
    int miso;
    int cs;
    int sck;
    int rst;
    int busy;
} lora_config;

typedef struct lora_handle_t {
    uint32_t freq;
    uint32_t rst;
    spi_device_handle_t spi;
} lora_handle;

int lora_init(const lora_config config, lora_handle* handle);
void lora_reset(const lora_handle* device);

void lora_standby(const lora_handle* device);
void lora_receive(const lora_handle* device);
void lora_transmit(const lora_handle* device, uint32_t timeout);

void lora_set_sync_word(const lora_handle* device, uint16_t sync_word);
void lora_set_byte_sync_word(const lora_handle* device, uint8_t sync_word);
uint16_t lora_get_sync_word(const lora_handle* handle);

void lora_set_packet_type(const lora_handle* device, uint8_t packet_type);
void lora_set_pa_config(
    const lora_handle* lora, uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_select);
void lora_set_mod_params(const lora_handle* handle, uint8_t sf, uint8_t bw, uint8_t cr);
void lora_set_frequency(lora_handle* device, uint32_t freq);
void lora_set_packet_params(const lora_handle* device, uint16_t preamble_length,
    uint8_t header_type, uint8_t payload_length, uint8_t crc_type, int8_t invert_iq);

void lora_set_buffer_base(const lora_handle* device, uint8_t tx_base, uint8_t rx_base);
void lora_write_tx_message(const lora_handle* device, const uint8_t* buf, size_t len);
void lora_get_rx_buffer_status(
    const lora_handle* device, uint8_t* payload_length, uint8_t* rx_start_buffer);
uint8_t lora_received_packet(const lora_handle* device);
uint8_t lora_read_packet(const lora_handle* device, uint8_t* data, uint8_t len);

void lora_get_packet_status(
    const lora_handle* handle, int8_t* rssi, int8_t* snr, int8_t* signal_rssi);
int8_t lora_get_packet_rssi(const lora_handle* device);
int8_t lora_get_rssi_inst(const lora_handle* device);

void lora_set_dio_irq_params(const lora_handle* handle, uint16_t irq_mask);

uint16_t lora_get_irq_status(const lora_handle* device);
void lora_clear_irq_status(const lora_handle* device, uint16_t irq);
