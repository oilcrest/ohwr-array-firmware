/**
 *  \file
 *
 *  \brief Stream on serial port implementation file.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#include "serial.h"
#include "stream.h"

static result_t SerialStream_Read(Stream *pStream, void *buf, size_t length,
                                  size_t *rdlength) {
  if (rdlength)
    *rdlength = length;
  char *cbuf = (char *)buf;
  while (length--)
    *cbuf++ = Serial_Getc((uint8_t)(uintptr_t)pStream->pObj);
  return RESULT_OK;
}

static result_t SerialStream_Write(Stream *pStream, const void *buf,
                                   size_t length, size_t *wrlength) {
  Serial_Write((uint8_t)(uintptr_t)pStream->pObj, buf, length);
  if (wrlength)
    *wrlength = length;
  return RESULT_OK;
}

static result_t SerialStream_Flush(Stream *pStream) {
  Serial_Flush((uint8_t)(uintptr_t)pStream->pObj);
  return RESULT_OK;
}

static result_t SerialStream_Close(Stream *pStream) {
  return S("SerialStream_Close: Not implemented");
}

result_t SerialStream_Init(Stream *pStream, uint8_t port, int flags) {
  Stream_Init(pStream, (void *)(uintptr_t)port, flags);
  pStream->read = &SerialStream_Read;
  pStream->write = &SerialStream_Write;
  pStream->seek = NULL;
  pStream->flush = &SerialStream_Flush;
  pStream->close = &SerialStream_Close;
  return RESULT_OK;
}
