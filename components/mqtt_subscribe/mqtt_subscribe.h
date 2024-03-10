#ifndef __MQTT_SUBSCRIBE_H__
#define __MQTT_SUBSCRIBE_H__

#include <esp_event.h>
#include <esp_log.h>
#include "freertos/event_groups.h"
#include <mqtt_client.h>
#include <stdbool.h>

#define MQTT_SOUND_DATA_AVAILABLE BIT0
#define MQTT_MOTION_DATA_AVAILABLE BIT1
#define MQTT_IPADRESS_AVAILABLE BIT2

extern EventGroupHandle_t mqtteventgroup;
typedef struct Ip{
    char ip [64];
}Ip_Adress;

/**
 * @brief struct for saving the sensor values
 */
typedef struct sensor_values_t{
    char sound_sensor[16]; //stores sound_sensor value
    char motion_sensor[16]; //stores motion sensor value
}sensor_values;

/**
 * @brief function for initialising mqttclient
 * 
 */
esp_mqtt_client_handle_t mqttclient();

/**
 * @brief function for recieving mqtt sensor data
 * 
 * @return sensor_values
 */
sensor_values get_sensor_data();

/**
 * @brief Get the ip object
 * 
 * @return char 
 */
Ip_Adress get_ip();

#endif