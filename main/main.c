#include "config.h"
#include "lora/lora.h"
#include "oled/oled.h"

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
    .busy = GPIO_NUM_4,
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

            int8_t rssi;
            lora_get_packet_status(&lora, &rssi, NULL, NULL);
            printf("Recevied: %s | RSSI: %d\n", buf, rssi);
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

    lora_set_byte_sync_word(&lora, 0x34);
    uint16_t sync_word = lora_get_sync_word(&lora);
    ESP_LOGI("LoRa", "Get Sync word: %04X", sync_word);

    oled_init(oled_dev, &oled);

    lora_message message = {};
    oled_clear_display(&oled, false);
    render_oled(&lora, &oled, &message);
    oled_display(&oled);

    xTaskCreate(&task_rx, "task_rx", 2048, NULL, 5, NULL);
}
