#pragma once
#include <driver/spi_master.h>
#include <stdint.h>

typedef struct lora_device_t {
    uint64_t freq;
    int mosi;
    int miso;
    int cs;
    int sck;
    int rst;
    spi_device_handle_t __spi;
} lora_device;

int lora_init(lora_device* device);
void lora_idle(const lora_device* device);
void lora_sleep(const lora_device* device);
void lora_receive(const lora_device* device);
void lora_reset(const lora_device* device);

void lora_set_frequency(lora_device* device, uint64_t frequency);

void lora_set_sync_word(const lora_device* device, int sw);
int lora_receive_packet(const lora_device* device, uint8_t* buf, int size);
int lora_received(const lora_device* device);
int lora_packet_rssi(const lora_device* device);
void lora_dump_registers(const lora_device* devive);
