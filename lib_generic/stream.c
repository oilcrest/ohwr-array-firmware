#include "stream.h"
#include "printf.h"
#include <stdarg.h>

#define PSTREAM ((Stream *)pArg)

#include "debug.h"

static void Stream_PutChar(void *pArg, char c) {
  if ((PSTREAM->flags & STREAM_MODE_TEXT) && c == '\n')
    Stream_PutChar(pArg, '\r');
  if (PSTREAM->printfbufPos >= PRINTF_BUF_LENGTH) {
    Stream_Write(PSTREAM, PSTREAM->printfbuf, PSTREAM->printfbufPos, NULL);
    PSTREAM->printfbufPos = 0;
    Stream_Flush(PSTREAM);
  }
  PSTREAM->printfbuf[PSTREAM->printfbufPos++] = c;
}

#undef PBUF

result_t Stream_Printf_P(Stream *pStream, immutable_str fmt, ...) {
  // make sure that the buffer is not used
  Stream_Flush(pStream);

  // initialize the buffer
  pStream->printfbufPos = 0;

  va_list ap;
  va_start(ap, fmt);
  kvprintf_P(fmt, &Stream_PutChar, pStream, ap);
  va_end(ap);
  if (pStream->printfbufPos > 0)
    Stream_Write(pStream, pStream->printfbuf, pStream->printfbufPos, NULL);
  return RESULT_OK;
}

#ifdef UNITTEST

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// test driver 1 definition starts here

#define MEMFILE_MAX_LENGTH 1048576

typedef struct MemFile_struct {
  uint8_t data[MEMFILE_MAX_LENGTH];
  size_t length;
} MemFile;

static MemFile file;

static result_t MemDriver_Read(Stream *pStream, void *buf, size_t length,
                               size_t *rdsize) {
  MemFile *pMemFile = (MemFile *)pStream->pObj;
  size_t available = pMemFile->length - pStream->offset;
  if (available < length)
    length = available;
  memcpy(buf, pMemFile->data + pStream->offset, length);
  pStream->offset += length;
  if (rdsize)
    *rdsize = length;
  return RESULT_OK;
}

static result_t MemDriver_Write(Stream *pStream, const void *buf, size_t length,
                                size_t *wrsize) {
  MemFile *pMemFile = (MemFile *)pStream->pObj;
  result_t res = RESULT_OK;
  size_t available = MEMFILE_MAX_LENGTH - pStream->offset;
  if (length > available) {
    res = S("MemDriver_Write: No space left on device");
    length = available;
  }
  memcpy(pMemFile->data + pStream->offset, buf, length);
  if (wrsize)
    *wrsize = length;
  return res;
}

static result_t MemDriver_Seek(Stream *pStream, unsigned long pos,
                               Stream_SeekMode whence) {
  MemFile *pMemFile = (MemFile *)pStream->pObj;
  switch (whence) {
  case STREAM_SEEK_CUR:
    pos += pStream->offset;
  case STREAM_SEEK_SET:
    pStream->offset = (pos > pMemFile->length) ? pMemFile->length : pos;
    break;
  case STREAM_SEEK_END:
    pStream->offset = pos > pMemFile->length ? 0 : pMemFile->length - pos;
    break;
  default:
    return S("MemDriver_Seek: Invalid seek mode");
  }
  return RESULT_OK;
}

static result_t MemDriver_Flush(Stream *pStream) { return RESULT_OK; }

static result_t MemDriver_Close(Stream *pStream) { return RESULT_OK; }

static result_t MemDriver_Open(Stream *pStream, MemFile *file) {
  pStream->flags = 0;
  pStream->pObj = file;
  pStream->offset = 0;
  pStream->read = &MemDriver_Read;
  pStream->write = &MemDriver_Write;
  pStream->seek = &MemDriver_Seek;
  pStream->flush = &MemDriver_Flush;
  pStream->close = &MemDriver_Close;
  return RESULT_OK;
}

// test driver 1 definition ends here

#include <stdlib.h>

void testStream(void) {
  result_t res;
  Stream stream;
  unsigned long pos;
  size_t l;

  uint8_t testdata[MEMFILE_MAX_LENGTH + 5];

  srand(0xDEADBABE);
  for (size_t i = 0; i < sizeof(testdata) / sizeof(testdata[0]); ++i)
    testdata[i] = (uint8_t)rand();

  res = MemDriver_Open(&stream, &file);

  res = Stream_Length(&stream, &pos);
  printf("{%s} %lu\n", res, pos);
  assert(res == RESULT_OK);
  assert(pos == 0);

  res = Stream_Write(&stream, testdata, 1237, &l);
  printf("{%s} %zu\n", res, l);
  assert(res == RESULT_OK);

  //	res = Stream_tell(&stream, &pos);
  //	assert(res == RESULT_OK);
  //	assert(pos == 1237);

  //	res = Stream_length(&stream, &pos);
  //	printf("{%s} %lu\n", res, pos);
  //	assert(res == RESULT_OK);
  //	assert(pos == 1237);

  Stream_Printf_P(&stream, S("abc"));
  Stream_Close(&stream);

  printf("testStream passed\n\n");
}

#endif // UNITTEST
