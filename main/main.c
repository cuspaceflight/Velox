#include "config.h"
#include "lora.h"
#include "oled.h"

#include <string.h>

#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

uint8_t buf[32];

lora_config lora_conf = {
    .freq = CONFIG_LORA_FREQ,
    .mosi = GPIO_NUM_10,
    .miso = GPIO_NUM_9,
    .sck  = GPIO_NUM_8,
    .cs   = GPIO_NUM_2,
    .rst  = GPIO_NUM_3,
};
lora_handle lora;

oled_config oled_dev = {
    .sda = GPIO_NUM_6,
    .scl = GPIO_NUM_7,
};
oled_handle oled;

void task_rx(void* p)
{
    int x;
    for (;;) {
        while (lora_received_packet(&lora)) {
            x      = lora_read_packet(&lora, buf, sizeof(buf));
            buf[x] = 0;

            int8_t rssi = lora_get_packet_rssi(&lora);

            lora_message* received_message = (lora_message*)((void*)buf);

            oled_clear_display(&oled, false);
            render_oled(&lora, &oled, received_message);
            oled_display(&oled);

            printf("Recevied: %s\n", buf);
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

    oled_init(oled_dev, &oled);

    lora_message message = {};
    oled_clear_display(&oled, false);
    render_oled(&lora, &oled, &message);
    oled_display(&oled);

    xTaskCreate(&task_rx, "task_rx", 2048, NULL, 5, NULL);
}
