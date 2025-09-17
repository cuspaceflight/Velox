#include "config.h"
#include "lora/lora.h"
#include "oled/oled.h"
#include "web_files.h"
#include "webserver.h"

#include <string.h>

#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

uint8_t buf[64];

lora_config lora_conf = {
    .freq = CONFIG_LORA_FREQ,
    .mosi = GPIO_NUM_10,
    .miso = GPIO_NUM_9,
    .sck  = GPIO_NUM_8,
    .cs   = GPIO_NUM_2,
    .rst  = GPIO_NUM_3,
    .busy = GPIO_NUM_4,
};
lora_handle lora;

oled_config oled_dev = {
    .sda = GPIO_NUM_6,
    .scl = GPIO_NUM_7,
};
oled_handle oled;

lora_message receive_message;

void task_rx(void* p)
{
    int read_len;
    for (;;) {
        while (lora_received_packet(&lora)) {
            read_len = lora_read_packet(&lora, buf, sizeof(buf));

            receive_message = *((lora_message*)((void*)buf));

            render_oled(&lora, &oled, receive_message);
            oled_display(&oled);
        }
        vTaskDelay(1);
    }
}

void app_main()
{
    if (!lora_init(lora_conf, &lora)) {
        ESP_LOGE("LoRa", "Failed to initialize");
        return;
    }
    ESP_LOGI("LoRa", "Started LoRa");

    lora_receive(&lora);

    lora_set_byte_sync_word(&lora, CONFIG_LORA_SYNC_WORD);

    oled_init(oled_dev, &oled);

    oled_clear_display(&oled, false);
    oled_display(&oled);

    setup_web_server();

    xTaskCreate(&task_rx, "task_rx", 4096, NULL, 5, NULL);
}
