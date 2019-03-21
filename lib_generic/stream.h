/**
 *  \file
 *
 *  \brief Common I/O stream interface.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *
 *  \note Read and write operations are blocking by default.
 *        Non-blocking I/O for streams is not defined yet.
 */

#ifndef _STREAM_H__
#define _STREAM_H__

#include "types.h"

/// When set in Stream.flags, causes LF to CR+LF conversion
/// in printf.
#define STREAM_MODE_TEXT 1

/// Not implemented yet, reserved for future use.
#define STREAM_MODE_NONBLOCKING 2

//#ifdef UNITTEST
#define STREAM_ENABLE_RUNTIME_CHECKS
//#endif

typedef enum Stream_SeekMode_enum {
  STREAM_SEEK_SET,
  STREAM_SEEK_CUR,
  STREAM_SEEK_END
} Stream_SeekMode;

/** I'd use two much smaller buffers and let
 *  one of them be filled when the other is written,
 *  but seems like there's some bug in some lower layer
 *  that on some of the avr serial ports causes the DMA
 *  to write less data to serial port
 *  than it's told to when the successive writes are
 *  too close in time. So this one should accumulate
 *  a typical printf'ed string.
 */
#define PRINTF_BUF_LENGTH 100

/** \brief Stream class.
 *
 *  A Non-Virtual Interface idiom is applied, i.e.:
 *  \li When implementing a concrete stream, the initialization function
 *      should set all members to proper values. Function pointers
 *      may be set to NULL if a particular operation is not implemented.
 *  \li No other specific functions should be created for concrete.
 *  \li Only functions defined for base class (i.e. from this header file)
 *      should be called inside code using Stream.
 */
typedef struct Stream_struct Stream;
struct Stream_struct {
  int flags;
  unsigned long offset;
  void *pObj;
  char printfbuf[PRINTF_BUF_LENGTH];
  size_t printfbufPos;
  /// See Stream_Read() for implementation reuqirements.
  result_t (*read)(Stream *pStream, void *buf, size_t length, size_t *rdlength);
  /// see Stream_Write() for implementation requirements.
  result_t (*write)(Stream *pStream, const void *buf, size_t length,
                    size_t *wrlength);
  result_t (*seek)(Stream *pStream, unsigned long pos, Stream_SeekMode whence);
  result_t (*flush)(Stream *pStream);
  result_t (*close)(Stream *pStream);
};

static inline void Stream_Init(Stream *pStream, void *pObj, int flags) {
  pStream->flags = flags;
  pStream->offset = 0;
  pStream->pObj = pObj;
  pStream->printfbufPos = 0;
}

/** \brief Read bytes from stream.
 *
 *  \param[in]  pStream  Stream to read from.
 *  \param[in]  buf      Buffer for read data.
 *  \param[in]  length   Number of bytes to read.
 *  \param[out] rdlength Number of bytes actually read.
 *
 *  \return     Either RESULT_OK or error message.
 *
 *  \note EOF condition is not considered as an error. The function returns
 * RESULT_OK and rdlength
 */
static inline result_t Stream_Read(Stream *pStream, void *buf, size_t length,
                                   size_t *rdlength) {
#ifdef STREAM_ENABLE_RUNTIME_CHECKS
  if (!pStream->read)
    return S("Stream_Read: Not readable");
#endif // STREAM_ENABLE_RUNTIME_CHECKS
  return (*pStream->read)(pStream, buf, length, rdlength);
}

/** \brief Write bytes to stream.
 *
 *  \param[in]  pStream  Stream to write to.
 *  \param[in]  buf      Data to be written.
 *  \param[in]  length   Number of bytes to write.
 *  \param[out] wrlength Number of bytes actually written (optional).
 *
 *  \return     Either RESULT_OK or error message.
 *
 *  \note With blocking write exactly \c length bytes of data are written or the
 * function returns with error (but \c wrlength is still set to correct value).
 */
static inline result_t Stream_Write(Stream *pStream, const void *buf,
                                    size_t length, size_t *wrlength) {
#ifdef STREAM_ENABLE_RUNTIME_CHECKS
  if (!pStream->write)
    return S("Stream_Write: Not writable");
#endif // STREAM_ENABLE_RUNTIME_CHECKS
  return (*pStream->write)(pStream, buf, length, wrlength);
}

static inline result_t Stream_Seek(Stream *pStream, unsigned long pos,
                                   Stream_SeekMode whence) {
#ifdef STREAM_ENABLE_RUNTIME_CHECKS
  if (!pStream->seek)
    return S("Stream_Seek: Not seekable");
#endif // STREAM_ENABLE_RUNTIME_CHECKS
  return (*pStream->seek)(pStream, pos, whence);
}

static inline result_t Stream_Tell(Stream *pStream, unsigned long *pos) {
  *pos = pStream->offset;
  return RESULT_OK;
}

static inline result_t Stream_Length(Stream *pStream, unsigned long *length) {
  unsigned long pos;
  result_t res;
  *length = 0;
  if ((res = Stream_Tell(pStream, &pos)) != RESULT_OK)
    return res;
  if ((res = Stream_Seek(pStream, 0, STREAM_SEEK_END)) != RESULT_OK)
    return res;
  if ((res = Stream_Tell(pStream, length)) != RESULT_OK) {
    Stream_Seek(pStream, 0, STREAM_SEEK_CUR);
    return res;
  }
  if ((res = Stream_Seek(pStream, pos, STREAM_SEEK_SET)) != RESULT_OK)
    return res;
  return RESULT_OK;
}

static inline result_t Stream_Flush(Stream *pStream) {
  return (*pStream->flush)(pStream);
}

static inline result_t Stream_Close(Stream *pStream) {
  return (*pStream->close)(pStream);
}

result_t Stream_Printf_P(Stream *pStream, immutable_str fmt, ...);

#endif // !_STREAM_H__
