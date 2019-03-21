/**
 *  \file
 *
 *  \brief Stream on ByteFifo header file.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#include "fifostream.h"

#define PFIFO ((ByteFifo *)pStream->pObj)

static result_t ByteFifoStream_Read(Stream *pStream, void *buf, size_t length,
                                    size_t *rdlength) {
  unsigned l = ByteFifo_Read(PFIFO, buf, length);
  if (rdlength)
    *rdlength = l;
  return RESULT_OK;
}

static result_t ByteFifoStream_Write(Stream *pStream, const void *buf,
                                     size_t length, size_t *wrlength) {
  unsigned l = ByteFifo_Write(PFIFO, buf, length);
  if (wrlength)
    *wrlength = l;
  return RESULT_OK;
}

static result_t ByteFifoStream_Flush(Stream *pStream) { return RESULT_OK; }

static result_t ByteFifoStream_Close(Stream *pStream) { return RESULT_OK; }

#undef PFIFO

result_t ByteFifoStream_Init(Stream *pStream, ByteFifo *pFifo, int flags) {
  pStream->pObj = (void *)pFifo;
  pStream->flags = flags;
  pStream->offset = 0;
  pStream->read = &ByteFifoStream_Read;
  pStream->write = &ByteFifoStream_Write;
  pStream->seek = NULL;
  pStream->flush = &ByteFifoStream_Flush;
  pStream->close = &ByteFifoStream_Close;
  return RESULT_OK;
}
