#include "url_encoding.h"

bool is_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

char *url_encode(const char *str)
{
    static const char *hex = "0123456789abcdef";
    static char encoded[1024];
    char *p = encoded;
    while (*str)
    {
        if (is_alphanumeric(*str) || *str == '-' || *str == '_' || *str == '.' || *str == '~')
        {
            *p++ = *str;
        }
        else
        {
            *p++ = '%';
            *p++ = hex[*str >> 4];
            *p++ = hex[*str & 15];
        }
        str++;
    }
    *p = '\0';
    return encoded;
}