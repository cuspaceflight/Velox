#include "oled.h"

#include "fonts.h"

#include "string.h"
#include <driver/i2c.h>
#include <driver/i2c_master.h>
#include <esp_log.h>
#include <stdio.h>

#define OLED_ADDR         0x3C
#define OLED_WIDTH        128
#define OLED_HEIGHT       32
#define OLED_SCL_SPEED_HZ 400000

#define OLED_XFR_TIMEOUT_MS 500

#define CONTROL_BYTE_CMD_SINGLE  0x80
#define CONTROL_BYTE_CMD_STREAM  0x00
#define CONTROL_BYTE_DATA_SINGLE 0xC0
#define CONTROL_BYTE_DATA_STREAM 0x40

#define CMD_SET_CONTRAST     0x81
#define CMD_DISPLAY_RAM      0xA4
#define CMD_DISPLAY_ALLON    0xA5
#define CMD_DISPLAY_NORMAL   0xA6
#define CMD_DISPLAY_INVERTED 0xA7
#define CMD_DISPLAY_OFF      0xAE
#define CMD_DISPLAY_ON       0xAF

#define CMD_SET_MEMORY_ADDR_MODE 0x20
#define CMD_SET_HORI_ADDR_MODE   0x00
#define CMD_SET_VERT_ADDR_MODE   0x01
#define CMD_SET_PAGE_ADDR_MODE   0x02
#define CMD_SET_COLUMN_RANGE     0x21
#define CMD_SET_PAGE_RANGE       0x22

#define CMD_SET_DISPLAY_START_LINE 0x40
#define CMD_SET_SEGMENT_REMAP_0    0xA0
#define CMD_SET_SEGMENT_REMAP_1    0xA1
#define CMD_SET_MUX_RATIO          0xA8
#define CMD_SET_COM_SCAN_MODE      0xC8
#define CMD_SET_DISPLAY_OFFSET     0xD3
#define CMD_SET_COM_PIN_MAP        0xDA

#define CMD_NOP 0xE3

#define CMD_SET_DISPLAY_CLK_DIV 0xD5
#define CMD_SET_PRECHARGE       0xD9
#define CMD_SET_VCOMH_DESELCT   0xD8

#define CMD_SET_CHARGE_PUMP 0x8D

#define CMD_HORIZONTAL_RIGHT  0x26
#define CMD_HORIZONTAL_LEFT   0x27
#define CMD_CONTINUOUS_SCROLL 0x29
#define CMD_DEACTIVATE_SCROLL 0x2E
#define CMD_ACTIVE_SCROLL     0x2F
#define CMD_VERTICAL          0xA3

#define TAG "Oled"

void i2c_write(const oled_handle* handle, uint8_t* buf, uint8_t length)
{
    i2c_master_transmit(handle->i2c_dev_handle, buf, length, OLED_XFR_TIMEOUT_MS);
}

void oled_setup(const oled_handle* handle)
{
    ESP_LOGV(TAG, "OLED_SETUP");

    uint8_t buf[40];
    uint8_t index = 0;

    buf[index++] = CONTROL_BYTE_CMD_STREAM;
    buf[index++] = CMD_DISPLAY_OFF;
    buf[index++] = CMD_SET_MUX_RATIO;
    buf[index++] = 0x1F;
    buf[index++] = CMD_SET_DISPLAY_OFFSET;
    buf[index++] = 0x00;
    buf[index++] = CMD_SET_DISPLAY_START_LINE;
    buf[index++] = CMD_SET_SEGMENT_REMAP_1;
    buf[index++] = CMD_SET_DISPLAY_CLK_DIV;
    buf[index++] = 0x80;
    buf[index++] = CMD_SET_COM_PIN_MAP;
    buf[index++] = 0x02;
    buf[index++] = CMD_SET_CONTRAST;
    buf[index++] = 0xFF;
    buf[index++] = CMD_SET_MEMORY_ADDR_MODE;
    buf[index++] = CMD_SET_PAGE_ADDR_MODE;
    buf[index++] = 0x0;
    buf[index++] = 0x10;
    buf[index++] = CMD_SET_CHARGE_PUMP;
    buf[index++] = 0x14;
    buf[index++] = CMD_DEACTIVATE_SCROLL;
    buf[index++] = CMD_DISPLAY_NORMAL;
    buf[index++] = CMD_DISPLAY_ON;

    i2c_write(handle, buf, index);
}

void oled_init(const oled_config config, oled_handle* handle)
{
    ESP_LOGV(TAG, "OLED_INIT");
    i2c_master_bus_config_t master_bus_config = {
        .i2c_port                     = I2C_NUM_0,
        .scl_io_num                   = config.scl,
        .sda_io_num                   = config.sda,
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_new_master_bus(&master_bus_config, &handle->i2c_handle);

    i2c_device_config_t i2c_dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = OLED_ADDR,
        .scl_speed_hz    = OLED_SCL_SPEED_HZ,
    };

    i2c_master_probe(handle->i2c_handle, OLED_ADDR, OLED_XFR_TIMEOUT_MS);
    i2c_master_bus_add_device(handle->i2c_handle, &i2c_dev_config, &handle->i2c_dev_handle);

    handle->context         = calloc(1, sizeof(oled_context));
    handle->context->width  = OLED_WIDTH;
    handle->context->height = OLED_HEIGHT;
    handle->context->pages  = OLED_PAGE_SIZE;

    for (uint8_t i = 0; i < handle->context->pages; i++) {
        memset(handle->context->page[i].segment, 0, OLED_PAGE_SEGMENT_SIZE);
    }

    oled_setup(handle);
}

void oled_display_image(
    const oled_handle* handle, uint8_t page, uint8_t segment, const uint8_t* image, uint8_t width)
{
    uint8_t column_low  = segment & 0x0F;
    uint8_t column_high = (segment >> 4) & 0x0F;

    uint8_t* out_buf;
    out_buf = malloc(width < 4 ? 4 : width + 1);

    uint8_t out_index    = 0;
    out_buf[out_index++] = CONTROL_BYTE_CMD_STREAM;
    out_buf[out_index++] = (0x00 + column_low);
    out_buf[out_index++] = (0x10 + column_high);
    out_buf[out_index++] = (0xB0 | page);

    i2c_write(handle, out_buf, out_index);

    out_buf[0] = CONTROL_BYTE_DATA_STREAM;
    memcpy(&out_buf[1], image, width);
    i2c_write(handle, out_buf, width + 1);

    free(out_buf);

    memcpy(&handle->context->page[page].segment[segment], image, width);
}

void oled_set_pixel(const oled_handle* handle, uint8_t y, uint8_t x, bool value)
{
    oled_page* page = &handle->context->page[3 - y / 8];
    if (value) {
        page->segment[x] |= 1 << (y % 8);
    } else {
        page->segment[x] &= ~(1 << (y % 8));
    }
}

void oled_display(const oled_handle* handle)
{
    for (uint8_t page = 0; page < handle->context->pages; page++) {
        oled_display_image(
            handle, page, 0, handle->context->page[page].segment, handle->context->width);
    }
}

void oled_set_text(const oled_handle* handle, uint8_t page, uint8_t segment, const char* str, ...)
{
    va_list args;
    va_start(args, str);
    char buf[25];
    int length = vsnprintf(buf, 25, str, args);
    va_end(args);

    ESP_LOGV(TAG, "OLED_SET_TEXT: STR: %.*s", length, buf);

    uint8_t index = 0;
    for (int k = 0; k < length; k++) {
        int x = segment + k * 6;

        unsigned char* font = console_font_5x8 + buf[index++] * 8;
        for (int i = 0; i < 5; i++) {
            uint8_t seg = 0;
            for (int j = 0; j < 8; j++) {
                seg |= ((font[j] >> (7 - i)) & 0x1) << (7 - j);
            }
            handle->context->page[3 - page].segment[x + i] = seg;
        }
    }
}

void oled_clear_display(const oled_handle* handle, bool invert)
{
    uint8_t* image;
    image = malloc(OLED_PAGE_SEGMENT_SIZE * sizeof(uint8_t));
    memset(image, invert ? 0xFF : 0x00, OLED_PAGE_SEGMENT_SIZE);

    for (uint8_t page = 0; page < handle->context->pages; page++) {
        oled_display_image(handle, page, 0, image, OLED_WIDTH);
    }
}
