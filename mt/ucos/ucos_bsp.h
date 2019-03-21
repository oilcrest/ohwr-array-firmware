/**
 *  \file
 *
 *  \brief Common uC/OS-II initialization interface.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _UCOS_BSP_H__
#define _UCOS_BSP_H__

#include "types.h"

/** \brief Initialize basic hardware functions needed to run uC/OS-II,
 *         create main task and hand control to uC/OS-II.
 *
 *  This function should be implemented as target specific initialization
 *  (usually in board/<i>boardname</i>/bsp.c, since BSP stands for Board
 *  Support Package), and transferring control to uC/OS-II.
 *
 *  \param[in]  mainTask      Task to run.
 *  \param[in]  pData         Pointer passed to mainTask.
 *  \param[in]  mainTaskStack Pointer to bottom of stack (initial
 *                            value of stack pointer).
 *  \param[in]  prio          Task priority.
 *
 *  \note  This function does not return (loops in OSStart()).
 */
void UCOS_Main(void (*mainTask)(void*), void *pData, uint8_t *mainTaskStack, uint8_t prio) __attribute__((noreturn));

#endif // !_UCOS_BSP_H__
