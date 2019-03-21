/** \file
 *
 *  \brief Line editor with command history and completion features.
 *
 *  It does not depend on the command set of underlying command processor
 *  nor on the specifics of the terminal device. Both should be provided
 *  as set of runtime interfaces.
 *  Also buffers for current line and history are provided from outside
 *  to make editor independent of memory allocation issues specific to
 *  particular platforms.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _EDITOR_H__
#define _EDITOR_H__

#include "fifo.h"
#include "terminal.h"
#include "types.h"

#define EDITOR_LINE_LENGTH_MIN 10
#define EDITOR_LINE_LENGTH_MAX 250

typedef struct CommandProcessor_struct {
  /** \brief Find all matches that complete given string.
   *
   *  Executes given callback for each matching entry, until there's no
   *  matches left, or the callback returns false.
   *
   *  \param[in]  pObj       Command processor object data.
   *  \param[in]  str        String to match.
   *  \param[in]  callback   Function to call for each match.
   *                         If the function returns TRUE, iteration continues.
   *                         If it returns false, iteration stops.
   *  \param[in]  pArg       Argument passed to the callback function.
   *  \param[in]  matchLen   Length of match buffer.
   *  \param[out] match      Buffer in which each found match will be placed
   *                         before call to callback function.
   *
   *  \return     RESULT_OK or error message.
   */
  result_t (*complete)(void *pObj, const char *str,
                       bool (*callback)(void *pArg), void *pArg,
                       uint8_t matchLen, char *match);

  /** \brief Parse and execute a complete command line.
   *
   *  \param[in]  pObj       Command processor object data.
   *  \param[in]  str        Command to execute.
   *  \param[in]  pOut       Implementation specific object used
   *                         to print command output.
   *
   *  \return     RESULT_OK or error message.
   */
  result_t (*execute)(void *pObj, const char *str, void *pOut);

  /** Object data */
  void *pObj;
} CommandProcessor;

/** \brief Editor configuration and variables.
 *
 *  \note  These are editor private data. The structure body
 *         is included in the header only to allow for allocating
 *         instances on stack or as static data.
 */
typedef struct Editor_struct {
  Terminal terminal;
  CommandProcessor cmdProc;
  ByteFifo history;
  char *match;
  char *line;
  unsigned maxLineLength;
  const char *prompt;
  uint8_t promptLen;
  unsigned lineLen;
  unsigned historyPosition;
} Editor;

/** \brief Initialize editor object.
 *
 *  \param[in]  pEditor          Editor structure to be initialized.
 *  \param[in]  pCmdProc         Command processor.
 *  \param[in]  pTerminal        Terminal.
 *  \param[in]  buffer           Buffer in which editor will keep the current
 * line and history. \param[in]  bufferSize       Size of buffer in bytes.
 *  \param[in]  maxLineLength    Maximum line length, including terminating NUL
 * character.
 *
 *  \return     RESULT_OK if editor initialized successfully. Error message
 * otherwise.
 *
 *  \note       Interfaces are copied internally so they are not needed after
 * initialization.
 *
 *  \note       Minimum bufferSize value is 3 * maxLineLength, which allows for
 * at least one entry in history at any time.
 */
result_t Editor_Init(Editor *pEditor, CommandProcessor *pCmdProc,
                     Terminal *pTerminal, char buffer[], unsigned bufferSize,
                     unsigned maxLineLength);

/** \brief Run editor.
 *
 *  \param[in]  pEditor   Initialized editor structure.
 *
 *  \return     RESULT_OK.
 */
result_t Editor_Run(Editor *pEditor);

/** \brief Print editor history to its terminal.
 *
 *  \param[in]  pEditor   Initialized editor structure.
 */
result_t Editor_PrintHistory(Editor *pEditor);

/** \brief Clear editor history.
 *
 *  \param[in]  pEditor   Initialized editor structure.
 */
void Editor_ClearHistory(Editor *pEditor);

#endif // !_EDITOR_H__
