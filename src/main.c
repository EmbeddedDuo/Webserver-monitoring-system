/*
* Sources:
* Spiffs: https://esp32tutorials.com/esp32-esp-idf-spiffs-web-server/
* 
*/

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
#define CAN_SEND_NOTIFICATION BIT0

static const char *TAG = "SPIFFS";

// event group for sending notification
EventGroupHandle_t notification_eventgroup;

QueueHandle_t sensor_data_queue;

esp_mqtt_client_handle_t client;

char index_html[4096];

// function to load index.html in /data into index_html buffer using SPIFFS Filesystem
static void initi_web_page_buffer(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    // initialize with settings above and mount SPIFFS
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


    // set all values in index_html buffer to 0
    memset((void *)index_html, 0, sizeof(index_html));

    // check if index.html exists
    struct stat st;
    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        return;
    }

    //open index.html and read it to index_html buffer
    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(index_html, st.st_size, 1, fp) == 0)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
}

// uri handler for getting root page
esp_err_t send_web_page(httpd_req_t *req)
{
    esp_err_t response = httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return response;
}

// uri handler for getting sensor data 
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

// uri handler for getting ip_address of Camera_Webserver
esp_err_t get_ipadress_handler(httpd_req_t *req)
{
    Ip_Address ip_adress = get_ip();
    httpd_resp_send(req, ip_adress.ip, strlen(ip_adress.ip));
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

httpd_uri_t get_ipadress = {
    .uri = "/ipaddress",
    .method = HTTP_GET,
    .handler = get_ipadress_handler,
    .user_ctx = NULL};

//setup http server
httpd_handle_t setup_server(void)
{   
    // wait for camera webserver ip before building http server
    xEventGroupWaitBits(mqtt_eventgroup, MQTT_IPADRESS_AVAILABLE, pdFALSE, pdTRUE, portMAX_DELAY);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &sensor_data);
        httpd_register_uri_handler(server, &get_ipadress);
    }

    return server;
}

//task to get the sensor values from Mqtt
void get_sensor_data_task(void *pvParameters)
{
    sensor_values recieve_data; 

    //pointer for endpoint character for strtol convertion if not number
    char *motion_endpoint;
    char *sound_endpoint;

    uint16_t motion_value;
    uint16_t sound_value;

    while (1)
    {   
        //waits for Sound and Motion Data to be recieved
        xEventGroupWaitBits(mqtt_eventgroup, MQTT_SOUND_DATA_AVAILABLE | MQTT_MOTION_DATA_AVAILABLE, pdTRUE, pdTRUE, portMAX_DELAY);
        recieve_data = get_sensor_data();

        //Overwrite Data in Queue that is used by get_sensor_data uri handler
        xQueueOverwrite(sensor_data_queue, &recieve_data);

        //Convertion from String to int
        motion_value = (uint16_t)strtol(recieve_data.motion_sensor, &motion_endpoint, 10);
        sound_value = (uint16_t)strtol(recieve_data.sound_sensor, &sound_endpoint, 10);


        // threshold value for sending notifications
        if (motion_value >= 492 || sound_value >= 150)
        {
            xEventGroupSetBits(notification_eventgroup, CAN_SEND_NOTIFICATION);
        }
        else if (motion_value <= 200 && sound_value <= 80)
        {
            xEventGroupClearBits(notification_eventgroup, CAN_SEND_NOTIFICATION);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//tasks for buzzer and whatsapp notification
void notification_task(void *pvParameters)
{

    //callback function that is transmitted in task parameters, is set in Main 
    void (*function)(void) = (void (*)(void))pvParameters;

    while (1)
    {   
        //wait that both tasks can send notifications 
        xEventGroupWaitBits(notification_eventgroup, CAN_SEND_NOTIFICATION, pdFALSE, pdFALSE, portMAX_DELAY);
        function();

        TickType_t function_delay = 0;

        //set delay for specific task that is configured in Kconfig
        if (strcmp(pcTaskGetName(NULL), "whatsapp_task") == 0)
        {
            function_delay = CONFIG_MESSAGE_DELAY_WHATSAPP;
        }
        if (strcmp(pcTaskGetName(NULL), "buzzer_task") == 0)
        {
            function_delay = CONFIG_MESSAGE_DELAY_BUZZER;
        }

        vTaskDelay(pdMS_TO_TICKS(function_delay));
    }
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();

    // Check for NVS initialization errors
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // initializing of buzzer and wifi
    init_buzzer();

    init_wifi();

    // initializing of mqtt
    client = mqttclient();

    notification_eventgroup = xEventGroupCreate();

    //create queue that delivers data from function that gets sensor data from mqtt to uri handler to display values
    sensor_data_queue = xQueueCreate(1, sizeof(sensor_values));

    xTaskCreate(get_sensor_data_task, "get_sensor_data_task", configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY, NULL);

    xTaskCreate(notification_task, "whatsapp_task", configMINIMAL_STACK_SIZE * 5, (void *)send_whatsapp_message, tskIDLE_PRIORITY, NULL);
    xTaskCreate(notification_task, "buzzer_task", configMINIMAL_STACK_SIZE * 5, (void *)alarm_buzzer, tskIDLE_PRIORITY, NULL);

    initi_web_page_buffer();
    setup_server();

    vTaskDelete(NULL);
}