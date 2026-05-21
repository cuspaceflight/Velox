#include "config.h"
#include "lora/lora.h"
#include "oled/oled.h"
#include "web_files.h"
#include "webserver.h"

#include <string.h>

#include "driver/spi_master.h"
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define STRLEN 512

uint8_t buf[200];
char strbuf[STRLEN];

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

// void task_tx(void* p)
// {
//     int counter = 0;
//     char message[255];
//     for (;;) {
//
//         int len = snprintf(message, 255, "Hello, World: %d\n", counter++);
//
//         lora_write_tx_message(&lora, (uint8_t*)message, len);
//         lora_set_packet_params(&lora, CONFIG_LORA_PREAMBLE_LENGTH, CONFIG_LORA_IMPLICIT_HEADER,
//         len,
//             CONFIG_LORA_CRC, CONFIG_LORA_INVERT_IQ);
//         lora_transmit(&lora, 0);
//         ESP_LOGI("SEND", "%.*s", len, message);
//
//         oled_set_text(&oled, 0, 0, message);
//         oled_display(&oled);
//
//         while (!(lora_get_irq_status(&lora) & LORA_IRQ_TX_DONE)) { }
//
//         lora_clear_irq_status(&lora, LORA_IRQ_ALL);
//         vTaskDelay(100);
//     }
// }

esp_err_t send_web_data(httpd_req_t* req)
{
    ESP_LOGI("GENERAL", "Web Data Send");
    format_json(receive_message, strbuf, STRLEN);
    return httpd_resp_send(req, strbuf, HTTPD_RESP_USE_STRLEN);
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

    setup_web_server(send_web_data);

    // xTaskCreate(&task_tx, "task_tx", 4096, NULL, 5, NULL);
    xTaskCreate(&task_rx, "task_rx", 4096, NULL, 5, NULL);
}
