/**
 *  \file
 *
 *  \brief Stream on serial port header file.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _SERIALSTREAM_H__
#define _SERIALSTREAM_H__

#include "serial.h"
#include "stream.h"

/** \brief Initialize a stream to use given serial port.
 *
 *  \param[in]  pStream  Stream to initialize.
 *  \param[in]  port     Serial port to read from and/or write to.
 *  \param[in]  flags    Stream options.
 *
 *  \return     Either RESULT_OK or error message.
 */
result_t SerialStream_Init(Stream *pStream, uint8_t port, int flags);

#endif // _SERIALSTREAM_H__
