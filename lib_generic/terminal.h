#ifndef _TERMINAL_H__
#define _TERMINAL_H__

#include "types.h"

/** \brief Terminal interface.
 *
 *  Printable characters are those of codes 0x20 to 0xFF, inclusive.
 *  Other codes, i.e. 0x00 .. 0x1F, are reserved for control characters.
 *  Control characters are defined as TERMINAL_* (see appropriate
 *  definitions below in this file) and implement some special functions
 *  that do not require parameters and do not return anything.
 */
typedef struct Terminal_struct {
  /** \brief Read one character from console input.
   *
   *  \param[in] pObj     Object data.
   *  \param[in] timeout  Time after which the timeout error should be reported.
   *                      This value is measured in implementation specific
   * units.
   *
   *  \return    Read character.
   *
   *  \note Read should be performed as blocking and unbufferred, without
   * echoing.
   *
   *  \note Returned character is either a printable character or one of the
   *        following control codes:
   *        TERMINAL_ERROR, TERMINAL_BS, TERMINAL_HINT,
   *        TERMINAL_UP, TERMINAL_DOWN, TERMINAL_LEFT, TERMINAL_RIGHT,
   *        TERMINAL_HOME, TERMINAL_END
   */
  uint8_t (*getChar)(void *pObj, uint16_t timeout);

  /** \brief Write a single character to console output.
   *
   *  \param[in] pObj  Object data.
   *  \param[in] c     Character to output.
   *
   *  \note c may be either a printable character or one of the
   *        following control codes (command requests):
   *        TERMINAL_SAVE, TERMINAL_RESTORE, TERMINAL_CR, TERMINAL_ERASE,
   *        TERMINAL_LEFT, TERMINAL_RIGHT
   */
  void (*putChar)(void *pObj, uint8_t c);

  /** \brief Write a formatted string to console output.
   *
   *  \param[in] pObj     Internal console object data.
   *  \param[in] fmt      Format string.
   *  \param[in] ...      Format arguments.
   *
   *  \note Terminal control characters shouldn't be used neither
   *        in format string nor in %s strings, so it's implementation
   *        specific whether they are interpreted or just passed
   *        to underlying device with no change.
   */
  result_t (*printf_P)(void *pObj, immutable_str fmt, ...);

  /** \brief Get current cursor position.
   *
   *  \param[in]  pObj  Object data.
   *  \param[out] pX    Column number, 1 is the leftmost column.
   *  \param[out] pY    Row number, 1 is the upmost row.
   */
  result_t (*getCursorPosition)(void *pObj, unsigned *pX, unsigned *pY);

  /** \brief Set current cursor position.
   *
   *  \param[in]  pObj  Object data.
   *  \param[out] x     Column number, 1 is the leftmost column.
   *  \param[out] y     Row number, 1 is the upmost row.
   */
  result_t (*setCursorPosition)(void *pObj, unsigned x, unsigned y);

  /** Object data, passed as pObj to all functions. */
  void *pObj;
} Terminal;

/// Error while waiting for character to read
#define TERMINAL_ERROR 0x00

/// Backspace
#define TERMINAL_BS 0x08

/// Hint request from user
#define TERMINAL_HINT 0x09

/// New line
#define TERMINAL_NEWLINE 0x0D

/// Erase from the current cursor position to the end of line
#define TERMINAL_ERASE 0x0E

/// Save cursor
#define TERMINAL_SAVE 0x10

/// Restore cursor previously saved with TERMINAL_SAVE
#define TERMINAL_RESTORE 0x11

/// Move to beginning of line
#define TERMINAL_HOME 0x18

/// Move to end of line
#define TERMINAL_END 0x19

/// Move up
#define TERMINAL_UP 0x1C

/// Move down
#define TERMINAL_DOWN 0x1D

/// Move left
#define TERMINAL_LEFT 0x1E

/// Move right
#define TERMINAL_RIGHT 0x1F

#endif // !_TERMINAL_H__
