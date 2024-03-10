/*
* sources: https://github.com/espressif/esp-idf/blob/v5.0/examples/protocols/mqtt/tcp/main/app_main.c
* and course files
*/


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

// mqtt event group handler for receiving sound data, motion data and ipadress
extern EventGroupHandle_t mqtt_eventgroup;
typedef struct Ip{
    char ip [64];
}Ip_Address;

/**
 * @brief struct for saving the sensor values
 * 
 *   - sound_sensor: value for sound_sensor
 *   - motion_sensor: value for sound_sensor
 */
typedef struct sensor_values_t{
    char sound_sensor[16];
    char motion_sensor[16];
}sensor_values;

/**
 * @brief function for initialising mqttclient
 * @return
 *     - handler for mqtt
 */
esp_mqtt_client_handle_t mqttclient();

/**
 * @brief function for recieving mqtt sensor data
 * 
 * @return 
 *    - sensor_values
 */
sensor_values get_sensor_data();

/**
 * @brief Get the ip object
 * 
 * @return 
 *   - Ip_Address struct 
 */
Ip_Address get_ip();

#endif