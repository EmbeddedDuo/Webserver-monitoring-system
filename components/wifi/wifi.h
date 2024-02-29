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

/**
 * @brief function for checking if wifi established
 * @return 
 *    - true
 *    - false
*/
bool check_wifi_established();

#endif