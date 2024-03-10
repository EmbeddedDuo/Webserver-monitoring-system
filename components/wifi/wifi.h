/*
* sources: https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c
*/

#ifndef __WIFI_H__
#define __WIFI_H__

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "stdio.h"
#include "esp_netif.h"

/**
 * @brief function for initialising wifi
 * 
 */
void init_wifi();

#endif