/**
 *  \file
 *
 *  \brief Functions for computing CRC-16 (CRC-16-IBM or ModBus).
 *
 */

#ifndef _CRC16_H__
#define _CRC16_H__

#include "types.h"

extern const uint8_t CRC16_tabH[] IMMUTABLE_MEM;
extern const uint8_t CRC16_tabL[] IMMUTABLE_MEM;

static inline uint16_t CRC16_Init(void) { return 0xFFFF; }

static inline uint16_t CRC16_Update(uint16_t crc, uint8_t b) {
  uint8_t i = (uint8_t)crc ^ b;
  return ((uint8_t)(crc >> 8) ^ READ_IMMUTABLE_BYTE(&CRC16_tabH[i])) |
         (READ_IMMUTABLE_BYTE(&CRC16_tabL[i]) << 8);
}

#endif // !_CRC16_H__
