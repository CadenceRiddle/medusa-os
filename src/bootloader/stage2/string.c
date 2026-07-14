#include "string.h"
#include "stdint.h"

// Finds the first matching character in a null-terminated string.
const char* strchr(const char* str, char chr){
    if (str == NULL){
        return NULL;
    }

    while (*str){
        if (*str == chr){
            return str;
        }
        ++str;
    }

    return NULL;
}

// Copies a null-terminated string, treating a null source as an empty string.
char* strcpy(char* dst, const char* src){

    char* origDst = dst;

    if (dst == NULL){
        return NULL;
    }

    if (src == NULL){
        *dst = '\0';
        return dst;
    }

    while (*src){
        *dst = *src;
        ++src;
        ++dst;
    }

    *dst = '\0';
    return origDst;

}

// Counts characters in a null-terminated string before the terminator.
unsigned strlen(const char* str){
    unsigned len = 0;
    while (*str){
        ++len;
        ++str;
    }

    return len;
}
