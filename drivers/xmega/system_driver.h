/**
 *  \file
 *
 *  \brief System control fuctions.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _SYSTEM_DRIVER_H__
#define _SYSTEM_DRIVER_H__

#include "types.h"

/** \brief Halt, possibly minimizing power consumption
 */
void System_Halt(void);

/** \brief Do what's necessary after the system is waken up.
 */
void System_WakeUp(void);

/** \brief Reboot system.
 */
void System_Reset(void);

/** \brief Get cause of last system reset.
 *
 *  \return Constant string with brief description of last reset cause,
 *          or NULL in case it's unknown.
 */
immutable_str System_LastResetCause(void);

/** \brief Clear last reset cause flags.
 */
void System_ClearLastResetCause(void);

#endif // !_SYSTEM_DRIVER_H__

