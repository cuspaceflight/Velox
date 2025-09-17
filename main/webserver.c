#include "webserver.h"

#include "web_files.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "string.h"

#define TAG "WEB"

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;
        ESP_LOGI(TAG, "Station join, AID:%d", event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*)event_data;
        ESP_LOGI(TAG, "Station leave, AID:%d", event->aid);
    }
}

esp_err_t send_web_page(httpd_req_t* req)
{
    ESP_LOGI(TAG, "GET Request");
    return httpd_resp_send(req, main_page, HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t uri_get
    = { .uri = "/", .method = HTTP_GET, .handler = send_web_page, .user_ctx = NULL };

void setup_web_server()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_WEB_WIFI_SSID,
            .ssid_len = strlen(CONFIG_WEB_WIFI_SSID),
            .channel = CONFIG_WEB_WIFI_CHANNEL,
            .password = CONFIG_WEB_WIFI_PASSWORD,
            .max_connection = CONFIG_WEB_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = true,
            },
        },
    };
    if (strlen(CONFIG_WEB_WIFI_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Init WiFi: SSID: %s | PASS: %s | CHAN: %d | MAX_CONN: %d", CONFIG_WEB_WIFI_SSID,
        CONFIG_WEB_WIFI_PASSWORD, CONFIG_WEB_WIFI_CHANNEL, CONFIG_WEB_MAX_STA_CONN);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Adding URI handlers");
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &uri_get));
    }
    ESP_LOGI(TAG, "Setup http Server");
}
