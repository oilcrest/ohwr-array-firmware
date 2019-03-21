/**
 *  \file
 *
 *  \brief Command processor implementation.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *
 *  \see command.h for info on how to build command tree.
 */

#ifndef _CMDPROC_H__
#define _CMDPROC_H__

#include "command.h"
#include "editor.h"
#include "types.h"

/** \brief Find all matches that complete given string.
 *
 *  Executes given callback for each matching entry, until there's no
 *  matches left, or the callback returns FALSE.
 *
 *  \param[in]  pObj       Pointer to CmdProc structure.
 *  \param[in]  str        String to match.
 *  \param[in]  callback   Function to call for each match.
 *                         If the function returns TRUE, iteration continues.
 *                         If it returns FALSE, iteration stops.
 *  \param[in]  pArg       Argument passed to the callback function.
 *  \param[in]  matchLen   Length of match buffer.
 *  \param[out] match      Buffer in which found match will be placed.
 *
 *  \return     RESULT_OK or error message.
 */
result_t CmdProc_Complete(void *pObj, const char *str,
                          bool (*callback)(void *pArg), void *pArg,
                          uint8_t matchLen, char *match);

/** \brief Parse and execute a complete command line.
 *
 *  \param[in]  pObj       Pointer to CmdProc structure.
 *  \param[in]  str        Command to execute.
 *  \param[in]  pOut       Implementation specific object used
 *                         to print command output.
 *
 *  \return     RESULT_OK or error message.
 */
result_t CmdProc_Execute(void *pObj, const char *str, void *pOut);

/** \brief Initialize Command Processor.
 *
 *  \param[in]  pCmdProc         Command Processor to be initialized.
 *  \param[in]  commandArray     Root array of commands.
 *
 *  \return     RESULT_OK or error message.
 */
result_t CmdProc_Init(CommandProcessor *pCmdProc, Command commands[]);

#endif // !_CMDPROC_H__
