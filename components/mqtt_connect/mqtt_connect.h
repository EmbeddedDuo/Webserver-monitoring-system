#ifndef __MQTT_CONNECT_H__
#define __MQTT_CONNECT_H__

#include "esp_event.h"
#include <esp_log.h>
#include <mqtt_client.h>

/**
 * @brief function for initialising mqttclient
 * 
 */
esp_mqtt_client_handle_t mqttclient();

#endif