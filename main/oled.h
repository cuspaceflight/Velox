#pragma once

typedef struct oled_device_t {
    int sda;
    int scl;
    int __i2c_port;
} oled_device;

void oled_init(oled_device* device);
