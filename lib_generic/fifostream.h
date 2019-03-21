/**
 *  \file
 *
 *  \brief Stream on ByteFifo header file.
 *
 *  \note Especially for use in unittests.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _FIFOSTREAM_H__
#define _FIFOSTREAM_H__

#include "fifo.h"
#include "stream.h"

/** \brief Setup stream to use existing ByteFifo object.
 *
 *  \param[in] pStream   Stream to initalize.
 *  \param[in] pFifo     Already initialized ByteFifo.
 *  \param[in] flags     Stream flags.
 *
 *  \return    Either RESULT_OK or error message.
 */
result_t ByteFifoStream_Init(Stream *pStream, ByteFifo *pFifo, int flags);

#endif // !_FIFOSTREAM_H__
