#ifndef _CPU_AVR_ATXMEGA128A1_UCOS_MT_H__
#define _CPU_AVR_ATXMEGA128A1_UCOS_MT_H__

#ifdef _MT_H__
#error "Other mt.h already included!"
#endif

#define _MT_H__

#include "ucos_ii.h"
#include <avr/interrupt.h>

// atomic expression

#if OS_CRITICAL_METHOD == 3
#define MT_ATOMIC_EXPR(_expr)                                                  \
  do {                                                                         \
    OS_CPU_SR cpu_sr = 0;                                                      \
    OS_ENTER_CRITICAL();                                                       \
    (_expr);                                                                   \
    OS_EXIT_CRITICAL();                                                        \
  } while (0)
#endif

// semaphore

typedef OS_EVENT *MT_SemType;

#define MT_SEM_DECLARE(_name) MT_SemType _name

#define MT_SEM_INIT(_name, _cnt)                                               \
  ({                                                                           \
    MT_SemType tmp;                                                            \
    tmp = OSSemCreate(_cnt);                                                   \
    if (tmp)                                                                   \
      (_name) = tmp;                                                           \
    !!tmp;                                                                     \
  })

#define MT_SEM_PEND(_name, _timeout)                                           \
  ({                                                                           \
    INT8U err;                                                                 \
    OSSemPend((_name), (_timeout), &err);                                      \
    err == OS_NO_ERR;                                                          \
  })

#define MT_SEM_POST(_name) ({ OSSemPost(_name) == OS_NO_ERR; })

// timed wait

#define MT_SLEEPMS(_miliseconds)                                               \
  do {                                                                         \
    uint16_t ms = (_miliseconds);                                              \
    OSTimeDlyHMSM(0, 0, ms / 1000, ms % 1000);                                 \
  } while (0)

// interrupt

#define MT_ISR(_name)                                                          \
  void _name##_handler(void);                                                  \
  ISR(_name, ISR_NAKED) {                                                      \
    __asm__ __volatile__("push r0               \n\t"                          \
                         "in r0, %0             \n\t"                          \
                         "push r0               \n\t"                          \
                         "push r1               \n\t"                          \
                         "clr r1                \n\t"                          \
                         "push r2               \n\t"                          \
                         "push r3               \n\t"                          \
                         "push r4               \n\t"                          \
                         "push r5               \n\t"                          \
                         "push r6               \n\t"                          \
                         "push r7               \n\t"                          \
                         "push r8               \n\t"                          \
                         "push r9               \n\t"                          \
                         "push r10              \n\t"                          \
                         "push r11              \n\t"                          \
                         "push r12              \n\t"                          \
                         "push r13              \n\t"                          \
                         "push r14              \n\t"                          \
                         "push r15              \n\t"                          \
                         "push r16              \n\t"                          \
                         "push r17              \n\t"                          \
                         "push r18              \n\t"                          \
                         "push r19              \n\t"                          \
                         "push r20              \n\t"                          \
                         "push r21              \n\t"                          \
                         "push r22              \n\t"                          \
                         "push r23              \n\t"                          \
                         "push r24              \n\t"                          \
                         "push r25              \n\t"                          \
                         "push r26              \n\t"                          \
                         "push r27              \n\t"                          \
                         "push r28              \n\t"                          \
                         "push r29              \n\t"                          \
                         "push r30              \n\t"                          \
                         "push r31              \n\t"                          \
                         "in r16, %1            \n\t"                          \
                         "push r16              \n\t"                          \
                         "in r16, %2            \n\t"                          \
                         "push r16              \n\t"                          \
                         "in r16, %3            \n\t"                          \
                         "push r16              \n\t"                          \
                         "in r16, %4            \n\t"                          \
                         "push r16              \n\t"                          \
                         "in r16, %5            \n\t"                          \
                         "push r16              \n\t"                          \
                         "lds r16, OSIntNesting \n\t"                          \
                         "inc r16               \n\t"                          \
                         "sts OSIntNesting, r16 \n\t"                          \
                         "cpi r16, 1            \n\t"                          \
                         "brne 1f               \n\t"                          \
                         "in r26, %7            \n\t"                          \
                         "in r27, %8            \n\t"                          \
                         "lds r28, OSTCBCur     \n\t"                          \
                         "lds r29, OSTCBCur+1   \n\t"                          \
                         "st Y+,r26             \n\t"                          \
                         "st Y+,r27             \n\t"                          \
                         "1:  call " #_name "_handler \n\t"                    \
                         "call OSIntExit        \n\t"                          \
                         "lds r26,OSTCBCur      \n\t"                          \
                         "lds r26,OSTCBCur+1    \n\t"                          \
                         "pop r16               \n\t"                          \
                         "out %5, r16           \n\t"                          \
                         "pop r16               \n\t"                          \
                         "out %4, r16           \n\t"                          \
                         "pop r16               \n\t"                          \
                         "out %3, r16           \n\t"                          \
                         "pop r16               \n\t"                          \
                         "out %2, r16           \n\t"                          \
                         "pop r16               \n\t"                          \
                         "out %1, r16           \n\t"                          \
                         "pop r31               \n\t"                          \
                         "pop r30               \n\t"                          \
                         "pop r29               \n\t"                          \
                         "pop r28               \n\t"                          \
                         "pop r27               \n\t"                          \
                         "pop r26               \n\t"                          \
                         "pop r25               \n\t"                          \
                         "pop r24               \n\t"                          \
                         "pop r23               \n\t"                          \
                         "pop r22               \n\t"                          \
                         "pop r21               \n\t"                          \
                         "pop r20               \n\t"                          \
                         "pop r19               \n\t"                          \
                         "pop r18               \n\t"                          \
                         "pop r17               \n\t"                          \
                         "pop r16               \n\t"                          \
                         "pop r15               \n\t"                          \
                         "pop r14               \n\t"                          \
                         "pop r13               \n\t"                          \
                         "pop r12               \n\t"                          \
                         "pop r11               \n\t"                          \
                         "pop r10               \n\t"                          \
                         "pop r9                \n\t"                          \
                         "pop r8                \n\t"                          \
                         "pop r7                \n\t"                          \
                         "pop r6                \n\t"                          \
                         "pop r5                \n\t"                          \
                         "pop r4                \n\t"                          \
                         "pop r3                \n\t"                          \
                         "pop r2                \n\t"                          \
                         "pop r1                \n\t"                          \
                         "pop r0                \n\t"                          \
                         "out %0, r0            \n\t"                          \
                         "pop r0                \n\t"                          \
                         "reti                  \n"                            \
                         :                                                     \
                         : "I"(_SFR_IO_ADDR(SREG)), "I"(_SFR_IO_ADDR(EIND)),   \
                           "I"(_SFR_IO_ADDR(RAMPD)), "I"(_SFR_IO_ADDR(RAMPX)), \
                           "I"(_SFR_IO_ADDR(RAMPY)), "I"(_SFR_IO_ADDR(RAMPZ)), \
                           "I"(_SFR_IO_ADDR(RAMPZ)), "I"(_SFR_IO_ADDR(SPL)),   \
                           "I"(_SFR_IO_ADDR(SPH)));                            \
  }                                                                            \
  void _name##_handler(void)

#endif // !_CPU_AVR_ATXMEGA128A1_MT_MT_H__
