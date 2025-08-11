#pragma once

#include "driver/i2c_master.h"

#define OLED_PAGE_SEGMENT_SIZE 128
#define OLED_PAGE_SIZE         4

typedef struct oled_config_t {
    int sda;
    int scl;
} oled_config;

typedef struct oled_page_t {
    uint8_t segment[OLED_PAGE_SEGMENT_SIZE];
} oled_page;

typedef struct oled_context_t {
    uint8_t width;
    uint8_t height;
    bool scroll_enabled;
    uint8_t scroll_start;
    uint8_t scroll_end;
    int8_t scroll_direction;
    uint8_t pages;
    oled_page page[OLED_PAGE_SIZE];
} oled_context;

typedef struct oled_handle_t {
    i2c_master_bus_handle_t i2c_handle;
    i2c_master_dev_handle_t i2c_dev_handle;
    oled_context* context;
} oled_handle;

void oled_init(const oled_config config, oled_handle* handle);

void oled_display_image(
    const oled_handle* handle, uint8_t page, uint8_t segment, const uint8_t* image, uint8_t width);

void oled_display(const oled_handle* handle);
void oled_clear_display(const oled_handle* handle, bool invert);
