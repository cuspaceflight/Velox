#pragma once

#include "lora/lora.h"
#include "oled/oled.h"

#include <math.h>

#include "esp_log.h"

static const char* json_resp = R"RAWSTRING(
{
"TIME": "%lu",
"ENTR": "%lu",
"GLAT": "%14.06f",
"GLON": "%15.06f",
"GALT": "%f",
"GSPD": "%f",
"BTEM": "%f",
"BPRE": "%f",
"BALT": "%f",
"ITEM": "%f",
"IACX": "%f",
"IACY": "%f",
"IACZ": "%f",
"IGYX": "%f",
"IGYY": "%f",
"IGYZ": "%f"
}
)RAWSTRING";

typedef struct lora_message_t {
    uint32_t timestamp;
    uint32_t entry;
    float gps_lat;
    float gps_lon;
    float gps_alt;
    float gps_speed;
    float bmp_temperature;
    float bmp_pressure;
    float bmp_altitude;
    float icm_temp;
    float icm_accel_x;
    float icm_accel_y;
    float icm_accel_z;
    float icm_gyro_x;
    float icm_gyro_y;
    float icm_gyro_z;
} lora_message;

void render_oled(const lora_handle* lora, const oled_handle* oled, const lora_message message)
{
    static float s_gps_lat = 0.0f, s_gps_lon = 0.0f, s_gps_alt = 0.0f;

    if (message.gps_lat != 0.0f) {
        s_gps_lat = message.gps_lat;
        s_gps_lon = message.gps_lon;
        s_gps_alt = message.gps_alt;
    }

    int8_t rssi = lora_get_packet_rssi(lora);
    oled_set_text(oled, 3, 15 * 6, "%dMHz", (lora->freq) / (uint32_t)1e6);

    oled_set_text(oled, 0, 0, "LAT : %-3d %2d %7.5f", (int)floorf(s_gps_lat / 10000),
        (int)floorf(s_gps_lat / 100.f) % 100, fmodf(s_gps_lat, 100.f));

    oled_set_text(oled, 1, 0, "LON : %-3d %2d %7.5f", (int)floorf(s_gps_lon / 10000),
        (int)floorf(s_gps_lon / 100.f) % 100, fmodf(s_gps_lon, 100.f));

    oled_set_text(oled, 2, 0, "ALT : %-7.3f", s_gps_alt);
    // oled_set_text(oled, 0, 0, "LAT : %-9.6f", s_gps_lat);
    // oled_set_text(oled, 1, 0, "LON : %-9.6f", s_gps_lon);
    // oled_set_text(oled, 0, 0, "Y   : %-5.3f", message.icm_accel_y);
    // oled_set_text(oled, 1, 0, "TEMP: %-5.3f", message.icm_temp);
    // oled_set_text(oled, 2, 0, "ALT : %-5.3f", message.bmp_altitude);
    oled_set_text(oled, 3, 0, "RSSI: %-3d", rssi);
}

void format_json(const lora_message message, char* str, int len)
{
    ESP_LOGI("GENERAL", "Format SEND");
    snprintf(str, len, json_resp, message.timestamp, message.entry, message.gps_lat,
        message.gps_lon, message.gps_alt, message.gps_speed, message.bmp_temperature,
        message.bmp_pressure, message.bmp_altitude, message.icm_temp, message.icm_accel_x,
        message.icm_accel_y, message.icm_accel_z, message.icm_gyro_x, message.icm_gyro_y,
        message.icm_gyro_z);
}
