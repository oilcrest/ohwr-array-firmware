#ifndef _TYPES_H__
#define _TYPES_H__

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef PGM_P immutable_str;

#define S(_t) PSTR(_t)
#define IMMUTABLE_MEM PROGMEM
#define READ_IMMUTABLE_WORD(_ptr) ((uint16_t)pgm_read_word(_ptr))
#define READ_IMMUTABLE_BYTE(_ptr) ((uint8_t)pgm_read_byte(_ptr))
#define READ_IMMUTABLE_PTR(_ptr) ((void *)(pgm_read_word(_ptr)))

#define IMMUTABLE_STR(_name) const char IMMUTABLE_MEM _name[]

typedef immutable_str result_t;

#define RESULT_OK ((result_t)0)
#define RESULT_ERROR S("Unknown error")

#endif // !_TYPES_H__
