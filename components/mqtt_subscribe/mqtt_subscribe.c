#include "mqtt_subscribe.h"

static const char *TAG = "MQTT";

EventGroupHandle_t mqtteventgroup = NULL;

sensor_values recieve_data;

Ip_Address ip_adress;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base =%s , event_id =%lu ", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, CONFIG_EXAMPLE_MQTT_TOPIC_FIRST, 0);
        esp_mqtt_client_subscribe(client, CONFIG_EXAMPLE_MQTT_TOPIC_SECOND, 0);
        esp_mqtt_client_subscribe(client, "monitoring-system/ip-address", 0);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        
        char topic[64];
        sprintf(topic,"%.*s",event->topic_len, event->topic);
        printf("TOPIC =%s \r \n ", topic);

        char data[64];
        sprintf(data,"%.*s",event->data_len, event->data);
        printf("DATA =%s \r \n ", data);

        if(strcmp(topic,CONFIG_EXAMPLE_MQTT_TOPIC_FIRST) == 0){
            strcpy(recieve_data.sound_sensor,data);
            xEventGroupSetBits(mqtteventgroup, MQTT_SOUND_DATA_AVAILABLE);
        }

        if(strcmp(topic,CONFIG_EXAMPLE_MQTT_TOPIC_SECOND) == 0){
            strcpy(recieve_data.motion_sensor,data);
            xEventGroupSetBits(mqtteventgroup, MQTT_MOTION_DATA_AVAILABLE);
        }

        if(strcmp(topic,"monitoring-system/ip-address") == 0){
            strcpy(ip_adress.ip, data);
            xEventGroupSetBits(mqtteventgroup, MQTT_IPADRESS_AVAILABLE);
        }

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t mqttclient()
{

    mqtteventgroup = xEventGroupCreate();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_EXAMPLE_MQTT_Broker};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));

    xEventGroupWaitBits(mqtteventgroup, MQTT_SOUND_DATA_AVAILABLE | MQTT_MOTION_DATA_AVAILABLE, pdFALSE, pdTRUE, portMAX_DELAY);

    return client;
}

sensor_values get_sensor_data()
{
    return recieve_data;
}

Ip_Address get_ip(){
    return ip_adress;
}