#include "fifo.h"
#include <string.h>

void ByteFifo_Init(ByteFifo *pFifo, uint8_t *buf, unsigned capacity) {
  pFifo->buf = buf;
  pFifo->capacity = capacity;
  ByteFifo_Clear(pFifo);
}

void ByteFifo_Clear(ByteFifo *pFifo) {
  pFifo->front = 0;
  pFifo->rear = 0;
  pFifo->full = false;
}

/*
bool ByteFifo_IsEmpty(ByteFifo *pFifo)
{
        return pFifo->front == pFifo->rear && !pFifo->full;
}

bool ByteFifo_IsFull(ByteFifo *pFifo)
{
        return pFifo->full;
}
*/

unsigned ByteFifo_Length(ByteFifo *pFifo) {
  /*    x
   * HIJABCDEFG  r = f, full = true
   *    x
   * ..........  r = f, full = false
   *    f   r
   * ...ABCD...  length = r - f = 4;
   *  r f
   * H..ABCDEFG  length = r + capacity - f = 2 + 10 - 4 = 8
   */
  if (pFifo->front == pFifo->rear)
    return pFifo->full ? pFifo->capacity : 0;
  if (pFifo->front < pFifo->rear)
    return pFifo->rear - pFifo->front;
  return pFifo->rear + pFifo->capacity - pFifo->front;
}

bool ByteFifo_Put(ByteFifo *pFifo, uint8_t b) {
  if (pFifo->full)
    return false;
  pFifo->buf[pFifo->rear] = b;
  if (++pFifo->rear == pFifo->capacity)
    pFifo->rear = 0;
  if (pFifo->rear == pFifo->front)
    pFifo->full = true;
  return true;
}

/*
    f  r                      f    r
....ABC...   +ab        = ....ABCab.  2/2
    f  r                   r  f
....ABC...   +abcd      = d...ABCabc  4/4
    f  r                   r  f
....ABC...   +abcdefgh  = defgABCabc, 7/8
r   f                        rf
....ABCDEF   +abc       = abc.ABCDEF, 3/3
r   f


*/

/// \todo Optimize
unsigned ByteFifo_Write(ByteFifo *pFifo, const void *buf, unsigned length) {
  unsigned wr = 0;
  while (length-- && ByteFifo_Put(pFifo, *((const uint8_t *)buf))) {
    ++buf;
    ++wr;
  }
  return wr;
}

int ByteFifo_Get(ByteFifo *pFifo) {
  if (ByteFifo_IsEmpty(pFifo))
    return BYTEFIFO_EOF;
  pFifo->full = false;
  uint8_t b = pFifo->buf[pFifo->front];
  if (++pFifo->front == pFifo->capacity)
    pFifo->front = 0;
  return b;
}

/// \todo Optimize
unsigned ByteFifo_Read(ByteFifo *pFifo, void *buf, unsigned length) {
  unsigned rd = 0;
  int s;
  while (length-- && (s = ByteFifo_Get(pFifo)) != BYTEFIFO_EOF) {
    *((uint8_t *)buf) = s;
    ++rd;
    ++buf;
  }
  return rd;
}

int ByteFifo_Peek(ByteFifo *pFifo, unsigned dist) {
  if (dist >= ByteFifo_Length(pFifo))
    return BYTEFIFO_EOF;
  if (pFifo->front + dist >= pFifo->capacity)
    return pFifo->buf[pFifo->front + dist - pFifo->capacity];
  else
    return pFifo->buf[pFifo->front + dist];
}

#ifdef UNITTEST

#include <assert.h>
#include <stdio.h>

void testByteFifo(void) {
#define BUF_LEN 34000
  ByteFifo fifo;
  static uint8_t buf[BUF_LEN];
  static uint8_t data[257];
  int i;
  int j;

  for (i = 0; i < 257; ++i)
    data[i] = (i + 37) & 0xFF;

  ByteFifo_Init(&fifo, buf, BUF_LEN);
  assert(ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == 0);
  assert(!ByteFifo_IsFull(&fifo));
  assert(ByteFifo_Get(&fifo) == BYTEFIFO_EOF);

  assert(ByteFifo_Put(&fifo, 0x55));
  assert(!ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == 1);
  assert(!ByteFifo_IsFull(&fifo));

  assert(ByteFifo_Get(&fifo) == 0x55);
  assert(ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == 0);
  assert(!ByteFifo_IsFull(&fifo));
  assert(ByteFifo_Get(&fifo) == BYTEFIFO_EOF);

  // check if put works
  for (i = 0; i < BUF_LEN - 1; ++i) {
    assert(ByteFifo_Put(&fifo, data[i % 257]));
    assert(!ByteFifo_IsEmpty(&fifo));
    assert(ByteFifo_Length(&fifo) == i + 1);
    assert(!ByteFifo_IsFull(&fifo));
  }
  assert(ByteFifo_Put(&fifo, data[i % 257]));
  assert(!ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == BUF_LEN);
  assert(ByteFifo_IsFull(&fifo));

  // check if put cannot write and failed write does not change state
  assert(!ByteFifo_Put(&fifo, data[i % 257]));
  assert(!ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == BUF_LEN);
  assert(ByteFifo_IsFull(&fifo));

  // check if peek works
  for (i = 0; i < BUF_LEN; ++i) {
    assert(ByteFifo_Peek(&fifo, i) == data[i % 257]);
  }
  assert(ByteFifo_Peek(&fifo, BUF_LEN) == BYTEFIFO_EOF);
  assert(ByteFifo_Peek(&fifo, BUF_LEN + 1) == BYTEFIFO_EOF);

  // check if peek does not modify state
  assert(!ByteFifo_Put(&fifo, data[i % 257]));
  assert(!ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == BUF_LEN);
  assert(ByteFifo_IsFull(&fifo));

  // check if get works
  for (i = 0; i < BUF_LEN - 1; ++i) {
    assert(ByteFifo_Get(&fifo) == data[i % 257]);
    assert(!ByteFifo_IsEmpty(&fifo));
    assert(ByteFifo_Length(&fifo) == BUF_LEN - i - 1);
    assert(!ByteFifo_IsFull(&fifo));
  }
  assert(ByteFifo_Get(&fifo) == data[i % 257]);
  assert(ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == 0);
  assert(!ByteFifo_IsFull(&fifo));

  assert(ByteFifo_Get(&fifo) == BYTEFIFO_EOF);
  assert(ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == 0);
  assert(!ByteFifo_IsFull(&fifo));

  // check if write works
  for (i = 0; i <= 257; ++i) {
    assert(ByteFifo_Write(&fifo, data, i) == i);
    assert((!ByteFifo_IsEmpty(&fifo) && i != 0) ||
           (ByteFifo_IsEmpty(&fifo) && i == 0));
    assert(ByteFifo_Length(&fifo) == (i * (i + 1)) / 2);
    assert(!ByteFifo_IsFull(&fifo));
  }

  // check if read works
  for (i = 0; i <= 257; ++i) {
    assert(ByteFifo_Read(&fifo, data, i) == i);
    for (j = 0; j < i; ++j)
      assert(data[j] == ((37 + (j % 257)) & 0xFF));
    assert((!ByteFifo_IsEmpty(&fifo) && i != 257) ||
           (ByteFifo_IsEmpty(&fifo) && i == 257));
    assert(ByteFifo_Length(&fifo) == (257 * 258 / 2) - (i * (i + 1)) / 2);
    assert(!ByteFifo_IsFull(&fifo));
  }

  assert(ByteFifo_Get(&fifo) == BYTEFIFO_EOF);
  assert(ByteFifo_IsEmpty(&fifo));
  assert(ByteFifo_Length(&fifo) == 0);
  assert(!ByteFifo_IsFull(&fifo));

  printf("ByteFifo tests passed\n\n");
#undef BUF_LEN
}

#endif // UNITTEST
