/**
 *  \file
 *
 *  \brief Byte Queue (FIFO).
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *
 *  \note Some operations are not thread-safe. When ByteFifo is shared between
 * threads or with an interrupt, user needs to lock calls to ByteFifo_*
 * functions using critical sections or mutexes.
 */

#ifndef _FIFO_H__
#define _FIFO_H__

#include "types.h"

#define BYTEFIFO_EOF (-1)

// why doesn't C have templates...
typedef struct ByteFifo_struct {
  uint8_t *buf;
  unsigned capacity;
  unsigned front;
  unsigned rear;
  bool full;
} ByteFifo;

void ByteFifo_Init(ByteFifo *pFifo, uint8_t *buf, unsigned capacity);

void ByteFifo_Clear(ByteFifo *pFifo);

static inline bool ByteFifo_IsEmpty(ByteFifo *pFifo) {
  return (pFifo)->front == (pFifo)->rear && !((pFifo)->full);
}

static inline bool ByteFifo_IsFull(ByteFifo *pFifo) { return (pFifo)->full; }

/**
 *  \brief Get number of bytes pending in queue.
 */
unsigned ByteFifo_Length(ByteFifo *pFifo);

/**
 *  \brief Add one byte at queue end.
 *
 *  \return \c true if put was successfull.
 *          \c false if queue was already full.
 */
bool ByteFifo_Put(ByteFifo *pFifo, uint8_t b);

/**
 *  \brief Add sequence of bytes at queue end.
 *
 *  \return Actual number of bytes written. If != \c length, more
 *          data could not be written because queue was full.
 */
unsigned ByteFifo_Write(ByteFifo *pFifo, const void *buf, unsigned length);

/**
 *  \brief Get one byte from queue.
 *
 *  \return Byte read or BYTEFIFO_EOF if queue is empty.
 */
int ByteFifo_Get(ByteFifo *pFifo);

unsigned ByteFifo_Read(ByteFifo *pFifo, void *buf, unsigned length);

int ByteFifo_Peek(ByteFifo *pFifo, unsigned dist);

#endif // !_FIFO_H__
