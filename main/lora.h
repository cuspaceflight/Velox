#pragma once
#include <driver/spi_master.h>
#include <stdint.h>

typedef struct lora_device_t {
    uint32_t freq;
    int mosi;
    int miso;
    int cs;
    int sck;
    int rst;
    int busy;
    spi_device_handle_t __spi;
} lora_device;

int lora_init(lora_device* device);
void lora_reset(const lora_device* device);

void lora_set_packet_type(const lora_device* device, uint8_t packet_type);
uint8_t lora_get_packet_type(const lora_device* device);

void lora_set_frequency(lora_device* device, uint32_t freq);

void lora_set_buffer_base(const lora_device* device, uint8_t tx_base, uint8_t rx_base);
void lora_get_buffer_status(
    const lora_device* device, uint8_t* payload_length, uint8_t* rx_start_buffer);

void lora_standby(const lora_device* device);
void lora_receive(const lora_device* device);

uint8_t lora_received_packet(const lora_device* device);
uint8_t lora_read_packet(const lora_device* device, uint8_t* data, uint8_t len);

uint16_t lora_get_irq_status(const lora_device* device);
void lora_clear_irq_status(const lora_device* device, uint16_t irq);
