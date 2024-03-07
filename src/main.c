#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <freertos/queue.h>
#include "driver/gpio.h"
#include "freertos/task.h"
#include <sys/stat.h>
#include "esp_system.h"
#include <esp_log.h>
#include <esp_http_server.h>
#include "esp_spiffs.h"

#include "wifi.h"
#include "mqtt_subscribe.h"
#include "whatsapp_messaging_http.h"
#include "buzzer.h"

#define INDEX_HTML_PATH "/spiffs/index.html"
#define CAN_SEND_WHATSAPP BIT0
#define BUZZER_GPIO 18

static const char *TAG = "SPIFFS";

EventGroupHandle_t eventgroup;

QueueHandle_t sensor_data_queue;

esp_mqtt_client_handle_t client;

char index_html[4096];

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
    sensor_values recieve_data;
    char buffer[100];

    if (xQueuePeek(sensor_data_queue, &recieve_data, 0) == pdTRUE)
    {
        snprintf(buffer, sizeof(buffer), "{\"sound_sensor\": %s, \"motion_sensor\": %s}", recieve_data.sound_sensor, recieve_data.motion_sensor);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "{\"sound_sensor\": couldnt recieve data, \"motion_sensor\": couldnt recieve data}");
    }

    httpd_resp_set_type(req, "application/json");
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

void get_sensor_data_task(void *pvParameters)
{
    while (1)
    {
        xEventGroupWaitBits(mqtteventgroup, MQTT_SOUND_DATA_AVAILABLE | MQTT_MOTION_DATA_AVAILABLE, pdTRUE, pdTRUE, portMAX_DELAY);
        sensor_values recieve_data = get_sensor_data();

        xQueueOverwrite(sensor_data_queue, &recieve_data);

        if (strtof(recieve_data.motion_sensor, NULL) >= 492.5 || strtof(recieve_data.sound_sensor, NULL) >= 492.5)
        {
            xEventGroupSetBits(eventgroup, CAN_SEND_WHATSAPP);
        }
        else if (strtof(recieve_data.motion_sensor, NULL) <= 200 || strtof(recieve_data.sound_sensor, NULL) <= 80)
        {
            xEventGroupClearBits(eventgroup, CAN_SEND_WHATSAPP);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void notification_task(void *pvParameters)
{
    gpio_reset_pin(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO,0);

    while (1)
    {
        xEventGroupWaitBits(eventgroup, CAN_SEND_WHATSAPP, pdTRUE, pdFALSE, portMAX_DELAY);
        
        gpio_set_level(BUZZER_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(BUZZER_GPIO,0);

        send_whatsapp_message();

        vTaskDelay(pdMS_TO_TICKS(20000));
    }
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

    init_wifi();

    client = mqttclient();

    eventgroup = xEventGroupCreate();

    sensor_data_queue = xQueueCreate(1, sizeof(sensor_values));

    xTaskCreate(get_sensor_data_task, "get_sensor_data_task", configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(notification_task, "notification_task", configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY, NULL);

    initi_web_page_buffer();
    setup_server();
    
    vTaskDelete(NULL);
}