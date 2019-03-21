#ifndef _CPU_AVR_ATXMEGA128A1_NOMT_MT_H__
#define _CPU_AVR_ATXMEGA128A1_NOMT_MT_H__

#ifdef _MT_H__
#error "Other mt.h already included!"
#endif

#define _MT_H__

#include <avr/interrupt.h>
#include <util/atomic.h>

// atomic expression

#define MT_ATOMIC_EXPR(_expr)                                                  \
  do {                                                                         \
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { (_expr); }                             \
  } while (0)

#define MT_CRITICAL_SECTION_END

// semaphore

typedef volatile uint16_t MT_SemType;

#define MT_SEM_DECLARE(_name) MT_SemType _name

#define MT_SEM_INIT(_name, _cnt)                                               \
  ({                                                                           \
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { (_name) = (_cnt); }                    \
    true;                                                                      \
  })

#define MT_SEM_PEND(_name, _timeout)                                           \
  ({                                                                           \
    MT_SemType _mtc;                                                           \
    do {                                                                       \
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { _mtc = (_name); }                    \
    } while (_mtc == 0);                                                       \
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { --(_name); }                           \
    true;                                                                      \
  })

#define MT_SEM_POST(_name)                                                     \
  ({                                                                           \
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { ++(_name); }                           \
    true;                                                                      \
  })

// interrupt

#define MT_ISR(_name) ISR(_name)

#endif // !_CPU_AVR_ATXMEGA128A1_NOMT_MT_H__
