#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */
#include <string.h>

#include "utils.h"


bool stringEquals(const char * str1, const char * str2) {
    return (strcmp(str1, str2) == 0);
}

uint16_t stringToInt(const char * str) {
    uint16_t val = 0;
    uint16_t tmp = 0;
    uint16_t shift = 0;
    while(*str != '\0') {
        tmp = *str - 0x30;
        if (shift != 0) {
            val *= 10;
        }
        val += tmp;
        shift++;
        str++;
    }
    return val;
}
