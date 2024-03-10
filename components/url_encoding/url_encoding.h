/*
* sources: https://esp32tutorials.com/esp32-esp-idf-send-messages-whatsapp/
*/

#ifndef __URL_ENCODING_H__
#define __URL_ENCODING_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief function for encoding any string for url convertion
 *
 * @param str the char* that will be encoded
 * @return
 *     - encoded char*
 *
 */
char *url_encode(const char *str);

#endif