#ifndef __MQTT_SUBSCRIBE_H__
#define __MQTT_SUBSCRIBE_H__

#include <esp_event.h>
#include <esp_log.h>
#include "freertos/event_groups.h"
#include <mqtt_client.h>
#include <stdbool.h>

#define MQTT_CONNECTED BIT0
#define MQTT_DATA_AVAILABLE BIT1

extern EventGroupHandle_t mqtteventgroup;

/**
 * @brief struct for saving the sensor values
 */
typedef struct sensor_values_t{
    char sound_sensor[8]; //stores sound_sensor value
    char motion_sensor[8]; //stores motion sensor value
}sensor_values;

/**
 * @brief function for initialising mqttclient
 * 
 */
esp_mqtt_client_handle_t mqttclient();

/**
 * @brief function for recieving mqtt sensor data
 * 
 */
sensor_values get_sensor_data();

#endif