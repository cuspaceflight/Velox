#pragma once

#include "lora/lora.h"
#include "oled/oled.h"

#include "esp_log.h"

static const char* json_resp = R"RAWSTRING(
{
"TIME": "%lu",
"ENTR": "%lu",
"GLAT": "%f",
"GLON": "%f",
"GALT": "%f",
"BPRE": "%f",
"BTEM": "%f",
"MACX": "%f",
"MACY": "%f",
"MACZ": "%f",
"MGYX": "%f",
"MGYY": "%f",
"MGYZ": "%f",
"MTEM": "%f"
}
)RAWSTRING";

typedef struct lora_message_t {
    uint32_t timestamp;
    uint32_t entry;
    float gps_lat;
    float gps_lon;
    float gps_alt;
    float gps_speed;
    float bmp_pressure;
    float bmp_temperature;
    float mpu_accel_x;
    float mpu_accel_y;
    float mpu_accel_z;
    float mpu_gyro_x;
    float mpu_gyro_y;
    float mpu_gyro_z;
    float mpu_temp;
} lora_message;

void render_oled(const lora_handle* lora, const oled_handle* oled, const lora_message message)
{
    int8_t rssi = lora_get_packet_rssi(lora);
    oled_set_text(oled, 0, 15 * 6, "%dMHz", (lora->freq) / (uint32_t)1e6);

    oled_set_text(oled, 0, 0, "LAT : %08.4f", message.gps_lat);
    oled_set_text(oled, 1, 0, "LON : %08.4f", message.gps_lon);
    oled_set_text(oled, 2, 0, "ALT : %08.4f", message.gps_alt);
    oled_set_text(oled, 3, 0, "RSSI: %-3d", rssi);

    oled_set_text(oled, 3, 10 * 6, "TEMP: %-03.1f", message.mpu_temp);
}

void format_json(const lora_message message, char* str, int len)
{
    ESP_LOGI("GENERAL", "Format SEND");
    snprintf(str, len, json_resp, message.timestamp, message.entry, message.gps_lat,
        message.gps_lon, message.gps_alt, message.bmp_pressure, message.bmp_temperature,
        message.mpu_accel_x, message.mpu_accel_y, message.mpu_accel_z, message.mpu_gyro_x,
        message.mpu_gyro_y, message.mpu_gyro_z, message.mpu_temp);
}
