/**
 *  \file
 *
 *  \brief Common interface for debug console.
 *
 *  \note To exclude all debug related functions and variables
 *  from application, rebuild all objects with DISABLE_DEBUG
 *  macro defined in app_cfg.h.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _DEBUG_H__
#define _DEBUG_H__

#include "app_cfg.h"
#include "types.h"

#ifndef DISABLE_DEBUG

/** \brief Initialize debugging features
 *
 *  \note Should be implemented by the driver intended to
 *  output debug messages.
 */
void Debug_Init(void);

/** \brief Write a null-terminated formatted string located in
 *  program address space to debug console device.
 *
 *  \param[in] fmt Format string.
 *
 *  \note Should be implemented by the driver intended to
 *  output debug messages.
 */
void Debug_Printf_P(immutable_str fmt, ...);

void Debug_Putc(char c);

#define DPRINTF(_fmt, ...)                                                     \
  do {                                                                         \
    Debug_Printf_P(PSTR(_fmt), ##__VA_ARGS__);                                 \
  } while (0)

#else

#define Debug_Init()
#define Debug_Printf_P(...)
#define DPRINTF(...)

#endif // DISABLE_DEBUG

#endif // _DEBUG_H__
