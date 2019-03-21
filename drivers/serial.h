/**
 *  \file
 *
 *  \brief Common interface for serial port(s).
 *
 *  \note All functions should implement blocking I/O.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _SERIAL_H__
#define _SERIAL_H__

#include "fifo.h"
#include "types.h"

#define SERIAL_USE_TX_DMA 1
#define SERIAL_USE_RX_DMA 2

result_t Serial_Init(uint8_t port, uint32_t baudrate, ByteFifo *pInFifo,
                     ByteFifo *pOutFifo, int options);

void Serial_Putc(uint8_t port, char k);

void Serial_Write(uint8_t port, const void *buf, size_t length);

void Serial_Flush(uint8_t port);

char Serial_Getc(uint8_t port);

#endif // !_SERIAL_H__
