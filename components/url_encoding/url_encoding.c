#include "url_encoding.h"

bool is_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

char *url_encode(const char *str)
{
    // all hexadecimical characters
    static const char *hex = "0123456789abcdef";

    //result buffer
    static char encoded[1024];

    //p current character for encoded string
    char *p = encoded;
    while (*str)
    {
        //check if character of string that should be converted is alphanumeric
        if (is_alphanumeric(*str) || *str == '-' || *str == '_' || *str == '.' || *str == '~')
        {
            *p++ = *str;
        }
        else
        {
            //if false then put % and turn character to 2 hexadecimical character
            *p++ = '%';
            *p++ = hex[*str >> 4];
            *p++ = hex[*str & 15];
        }
        str++;
    }

    // add nullcharacter to mark end of encoded string
    *p = '\0';
    return encoded;
}