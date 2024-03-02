#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include <sys/stat.h>
#include "esp_system.h"
#include <esp_log.h>
#include <esp_http_server.h>
#include "esp_spiffs.h"
#include <freertos/timers.h>

#include "wifi.h"
#include "mqtt_subscribe.h"
#include "whatsapp_messaging_http.h"

#define INDEX_HTML_PATH "/spiffs/index.html"

static const char *TAG = "SPIFFS";
TimerHandle_t whatsapp_messages_timer = NULL;
static const TickType_t delay_between_messages = CONFIG_MESSAGE_DELAY;

esp_mqtt_client_handle_t client;

bool allowed_to_send_whatsapp_messages = true;

char index_html[4096];

void whatsapp_messaging(TimerHandle_t pxTimer)
{
    allowed_to_send_whatsapp_messages = true;
}

static void initi_web_page_buffer(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    size_t total = 0, used = 0;
    if (esp_spiffs_info(conf.partition_label, &total, &used) == ESP_OK)
    {
        ESP_LOGI(TAG, "Partition size: total %d, used: %d", total, used);
    }
    else
    {
        ESP_LOGE(TAG, "Couldnt read partition info");
    }

    memset((void *)index_html, 0, sizeof(index_html));
    struct stat st;

    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        return;
    }

    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(index_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
}

esp_err_t send_web_page(httpd_req_t *req)
{
    esp_err_t response = httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return response;
}

esp_err_t get_data_handler(httpd_req_t *req)
{
    sensor_values recieve_data = get_sensor_data();

    // Prepare JSON response
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "{\"sound_sensor\": %s, \"motion_sensor\": %s}", recieve_data.sound_sensor, recieve_data.motion_sensor);
    ESP_LOGI("MQTT", "Daten sollen gesendet werden: %s", recieve_data.should_message_user ? "true" : "false");
    
    if (allowed_to_send_whatsapp_messages && recieve_data.should_message_user)
    {
        send_whatsapp_message();
        xTimerStart(whatsapp_messages_timer, 0);
        allowed_to_send_whatsapp_messages = false;
    }

    // Set response type as JSON
    httpd_resp_set_type(req, "application/json");
    // Send JSON response
    httpd_resp_send(req, buffer, strlen(buffer));

    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = send_web_page,
    .user_ctx = NULL};

httpd_uri_t sensor_data = {
    .uri = "/sensordata",
    .method = HTTP_GET,
    .handler = get_data_handler,
    .user_ctx = NULL};

httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &sensor_data);
    }

    return server;
}

void app_main()
{

    esp_err_t ret = nvs_flash_init(); // Initialize NVS Flash memory for the WIFI credentials

    // Check for NVS initialization errors
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); // Check for general initialization errors

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    whatsapp_messages_timer = xTimerCreate("message_timer", pdMS_TO_TICKS(delay_between_messages), pdFALSE, 0, whatsapp_messaging);

    init_wifi();

    while (!check_wifi_established())
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    client = mqttclient();
    initi_web_page_buffer();
    setup_server();
}