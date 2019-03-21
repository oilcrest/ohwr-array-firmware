/**
 *  \file
 *
 *  \brief Command Line Interface for TestBeam 2.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _CLI_H__
#define _CLI_H__

#include "command.h"
#include "stream.h"
#include "terminal.h"
#include "types.h"

/** \brief CLI entry point.
 *
 *  \param[in] pArg Stream object on which the console is open.
 */
void CLI_Run(Stream *pStream) __attribute__((noreturn));

#define CLI_PTERM ((Terminal *)pOut)
#define CLI_TPRINTF(fmt, ...)                                                  \
  (*CLI_PTERM->printf_P)(CLI_PTERM->pObj, S(fmt), ##__VA_ARGS__)
// format string is already IMMUTABLE
#define CLI_TPRINTFI(fmt, ...)                                                 \
  (*CLI_PTERM->printf_P)(CLI_PTERM->pObj, fmt, ##__VA_ARGS__)

#define CLI_TPRINTF_ASSERT(fmt, ...)                                           \
  do {                                                                         \
    result_t r = CLI_TPRINTF(fmt, ##__VA_ARGS__);                              \
    if (r != RESULT_OK)                                                        \
      return r;                                                                \
  } while (0)

#define CLI_TPRINTFI_ASSERT(fmt, ...)                                          \
  do {                                                                         \
    result_t r = CLI_TPRINTFI(fmt, ##__VA_ARGS__);                             \
    if (r != RESULT_OK)                                                        \
      return r;                                                                \
  } while (0)
#endif // !_CLI_H__
