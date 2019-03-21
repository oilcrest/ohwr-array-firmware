#ifndef __ASTRING_H__
#define __ASTRING_H__

#include "types.h"
#include <avr/pgmspace.h> // strlen_P, strncmp_P
#include <string.h>

int stricmp(const char *str1, const char *str2) __attribute__((pure));

int stricmp_P(const char *str1, immutable_str str2) __attribute__((pure));

int strnicmp(const char *str1, const char *str2, size_t count)
    __attribute__((pure));

int strnicmp_P(const char *str1, immutable_str str2, size_t count)
    __attribute__((pure));

#endif // __ASTRING_H__
