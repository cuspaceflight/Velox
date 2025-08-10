#include "lora.h"
#include "oled.h"

#include "driver/spi_master.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

uint8_t buf[32];

lora_device lora_dev = {
    .freq = 433E6,
    .mosi = GPIO_NUM_10,
    .miso = GPIO_NUM_9,
    .sck  = GPIO_NUM_8,
    .cs   = GPIO_NUM_2,
    .rst  = GPIO_NUM_3,
};

oled_device oled_dev = {
    .sda = GPIO_NUM_6,
    .scl = GPIO_NUM_7,
};

void task_rx(void* p)
{
    int x;
    for (;;) {
        while (lora_received_packet(&lora_dev)) {
            x      = lora_read_packet(&lora_dev, buf, sizeof(buf));
            buf[x] = 0;
            printf("Recevied: %s\n", buf);
        }
        vTaskDelay(1);
    }
}

void app_main()
{
    if (!lora_init(&lora_dev)) {
        ESP_LOGE("LoRa", "Failed to initialize");
        return;
    }
    ESP_LOGI("LoRa", "Started LoRa");

    lora_set_byte_sync_word(&lora_dev, 0x36);
    lora_receive(&lora_dev);

    oled_init(&oled_dev);

    xTaskCreate(&task_rx, "task_rx", 2048, NULL, 5, NULL);
}
