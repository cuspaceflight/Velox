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

    // lora_set_byte_sync_word(&lora, CONFIG_LORA_SYNC_WORD);
    lora_receive(&lora);

    oled_init(oled_dev, &oled);

    oled_clear_display(&oled, false);

    // oled_set_pixel(&oled, 0, 0, true);
    // oled_set_pixel(&oled, 0, 1, true);
    // oled_set_pixel(&oled, 1, 0, true);
    // oled_set_pixel(&oled, 1, 1, true);

    oled_set_text(&oled, 0, 0, "Hello, World!");
    oled_set_text(&oled, 0, 15 * 6, "%dMHz", (CONFIG_LORA_FREQ) / (uint32_t)1e6);
    oled_display(&oled);

    // uint8_t page[OLED_PAGE_SEGMENT_SIZE];
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 128 / 4; j++) {
    //         for (int k = 0; k < 4; k++) {
    //             page[j * 4 + k] = (j % 2 == 0) ? 0xF0 : 0x0F;
    //         }
    //     }
    //     oled_display_image(&oled, i, 0, page, OLED_PAGE_SEGMENT_SIZE);
    // }

    xTaskCreate(&task_rx, "task_rx", 2048, NULL, 5, NULL);
}
