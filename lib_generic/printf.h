/** \file
 *
 *  \brief Minimal universal *printf.
 *
 *  Based on FreeBSD printf, but optimized for 8-bit CPU.
 *  Also, some features are missing, some other added.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _PRINTF_H__
#define _PRINTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include <stdarg.h>

/** \brief Print formatted string.
 *
 *  \param[in]  fmt       Format string.
 *  \param[in]  putChar   Function to output a single character.
 *  \param[in]  pObj      Pointer passed as second argument to putChar.
 *  \param[in]  ap        Additional arguments.
 *
 *  \return     Number of characters printed.
 */
int kvprintf_P(immutable_str fmt, void (*putChar)(void *pObj, char c),
               void *pObj, va_list ap);

/** \brief Printf to character string.
 *
 *  Loads data from the given locations and writes them to the given character
 * string accorcing to the format parameter. At most buff_size characters are
 * written.
 *
 *  The character string will be terminated with a null character, unless
 * buf_size is zero.
 *
 *  \param[out] buffer      Printed string.
 *  \param[in]  bufSize     Maximum number of characters to write.
 *  \param[in]  format      Format string.
 *  \param[in]  ...         Additional arguments.
 */
int snprintf_P(char *buffer, int bufSize, immutable_str fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _PRINTF_H__
