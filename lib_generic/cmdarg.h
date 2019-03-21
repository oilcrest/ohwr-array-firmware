/**
 *  \file
 *
 *  \brief Set of functions for parsing command arguments.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _CMDARGS_H__
#define _CMDARGS_H__

#include "types.h"

/** \brief Skip whitespaces in given string.
 *
 *  \param[in]  str   Input string.
 *
 *  \return     Pointer to first non-whitespace character.
 */
const char *skipSpaces(const char *str);

/** \brief Convert string into integer.
 *
 *  Hexadecimal values are allowed using "0x" prefix.
 *
 *  \param[in,out] pStr   Pointer to first character of string to parse.
 *                        On output points to first character not being a part
 * of integer. \param[in]     min    Minimum integer value. \param[in]     max
 * Maximum integer value. \param[out]    val    Parsed integer.
 */
result_t parseInt(const char **pStr, int32_t min, int32_t max, int32_t *val);

result_t parseFloat(const char **pStr, float *val);

/** \brief Convert string "ON" or "OFF" into bool.
 *
 *  Hexadecimal values are allowed using "0x" prefix.
 *
 *  \param[in,out] pStr   Pointer to first character of string to parse.
 *                        On output points to first character not being a part
 * of parsed token. \param[out]    val    TRUE for "ON", FALSE for "OFF"r.
 */
result_t parseOnOff(const char **pStr, bool *val);

/** \brief Parse string, optionally quoted.
 *
 *  Unquoted string ends with first whitespace character or end of input string.
 *  Inside quoted string there are two escape sequences available: '\\' giving a
 * single backslash, and '\"' giving a double quote character. Other
 * two-character sequences starting with '\' are reserved and should not be
 * used.
 *
 *  \param[in,out] pStr   Pointer to first character of string to parse.
 *                        On output points to first character not being a part
 * of parsed string. \param[in]     len    Maximum string length. \param[out]
 * val    Parsed string.
 */
result_t parseString(const char **pStr, uint16_t len, char *val);

#endif // !_CMDARGS_H__
