#include "lora.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

uint8_t buf[32];

lora_device device = {
    .freq = 433E6,
    .mosi = GPIO_NUM_10,
    .miso = GPIO_NUM_9,
    .cs   = GPIO_NUM_2,
    .sck  = GPIO_NUM_8,
    .rst  = GPIO_NUM_3,
};

void task_rx(void* p)
{
    int x;
    for (;;) {
        lora_receive(&device);
        while (lora_received(&device)) {
            x      = lora_receive_packet(&device, buf, sizeof(buf));
            buf[x] = 0;
            printf("Recevied: %s\n", buf);
            lora_receive(&device);
        }
        vTaskDelay(1);
    }
}

void app_main()
{
    if (!lora_init(&device)) {
        ESP_LOGE("LORA", "Failed to initialize");
        return;
    }

    lora_set_frequency(&device, 433E6);

    lora_dump_registers(&device);
    // xTaskCreate(&task_rx, "task_rx", 2048, NULL, 5, NULL);
}
