#pragma once

#include "lora/lora.h"
#include "oled/oled.h"

typedef struct lora_message_t {
    float gps_lat;
    float gps_lon;
    float gps_alt;
} lora_message;

void render_oled(const lora_handle* lora, const oled_handle* oled, lora_message* message)
{
    int8_t rssi = lora_get_packet_rssi(lora);
    oled_set_text(oled, 0, 15 * 6, "%dMHz", (lora->freq) / (uint32_t)1e6);

    oled_set_text(oled, 0, 0, "LAT : %08.4f", message->gps_lat);
    oled_set_text(oled, 1, 0, "LON : %08.4f", message->gps_lon);
    oled_set_text(oled, 2, 0, "ALT : %08.4f", message->gps_alt);
    oled_set_text(oled, 3, 0, "RSSI: %-d", rssi);
}
