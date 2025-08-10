#include "oled.h"

#include <driver/i2c.h>
#include <driver/i2c_master.h>

#define OLED_ADDR 0x3C

#define CMD_SET_CONTRAST       0x81
#define CMD_ENTIRE_DISPLAY_OFF 0xA4
#define CMD_ENTIRE_DISPLAY_ON  0xA5
#define CMD_SET_NORMAL         0xA6
#define CMD_SET_INVERSE        0xA7
#define CMD_SET_DISPLAY_OFF    0xAE
#define CMD_SET_DISPLAY_ON     0xAF

void oled_init(oled_device* device)
{
    device->__i2c_port = I2C_NUM_0;
    i2c_config_t conf  = {
         .mode             = I2C_MODE_MASTER,
         .sda_io_num       = device->sda,
         .scl_io_num       = device->scl,
         .sda_pullup_en    = GPIO_PULLUP_DISABLE,
         .scl_pullup_en    = GPIO_PULLUP_DISABLE,
         .master.clk_speed = 1E5,
    };

    i2c_param_config(device->__i2c_port, &conf);

    i2c_driver_install(device->__i2c_port, conf.mode, 0, 0, 0);

    uint8_t buf[1] = { CMD_SET_DISPLAY_ON };
    i2c_master_write_to_device(
        device->__i2c_port, OLED_ADDR, buf, sizeof(buf), 1000 / portTICK_PERIOD_MS);
    buf[0] = CMD_ENTIRE_DISPLAY_ON;
    i2c_master_write_to_device(
        device->__i2c_port, OLED_ADDR, buf, sizeof(buf), 1000 / portTICK_PERIOD_MS);
}
