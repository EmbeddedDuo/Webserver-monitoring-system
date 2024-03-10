/*
* sources: https://esp32tutorials.com/esp32-esp-idf-send-messages-whatsapp/
*/

#ifndef __WHATSAPP_MESSAGING_HTTP__
#define __WHATSAPP_MESSAGING_HTTP__

#include "esp_http_client.h"
#include "esp_log.h"
#include "url_encoding.h"

/**
 * @brief function to send message over CallMeBot Api to phone number
 * 
 */
void send_whatsapp_message();

#endif