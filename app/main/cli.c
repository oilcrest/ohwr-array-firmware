/**
 *  \file
 *
 *  \brief Command line interface
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#include "cli.h"
#include "app_cfg.h"
#include "cmdproc.h"
#include "debug.h"
#include "editor.h"
#include "led.h"
#include "stream.h"
#include "vt100.h"

#ifndef EDITOR_LINE_SIZE
#error "EDITOR_LINE_SIZE not defined!"
#endif

#ifndef EDITOR_BUFFER_SIZE
#error "EDITOR_BUFFER_SIZE not defined!"
#endif

COMMAND_ARRAY_DECLARE(commandArray_ROOT);

// static char editorBuffer[EDITOR_BUFFER_SIZE]
// __attribute__((section(".extbss")));
static char editorBuffer[EDITOR_BUFFER_SIZE];
static Editor editor;

static void initEditor(Stream *pStream) {
  Terminal term;
  CommandProcessor cp;
  VT100_Init(&term, pStream);
  CmdProc_Init(&cp, commandArray_ROOT);
  Editor_Init(&editor, &cp, &term, editorBuffer, EDITOR_BUFFER_SIZE,
              EDITOR_LINE_SIZE);
}

void CLI_Run(Stream *pStream) {
  DPRINTF("OK.\n");
  for (;;) {
    Stream_Printf_P(
        pStream,
        S("\nHello, this is Switching Matrix Command Line Interface.\n"));
    initEditor(pStream);
    Editor_Run(&editor);
  }
}

// some standard commands

static void cmdHelpMain(Terminal *pTerminal, Command cmds[], Command *pDefault,
                        uint8_t nest) {
  uint8_t k;
  immutable_str name;
  while (!!(name = COMMAND_NAME(cmds))) {
    for (k = 0; k < nest; ++k)
      (*pTerminal->printf_P)(pTerminal->pObj, S("  "));
    (*pTerminal->printf_P)(pTerminal->pObj,
                           (cmds == pDefault) ? S("[%S]\n") : S("%S\n"), name);
    if (!COMMAND_ISLEAF(cmds))
      cmdHelpMain(pTerminal, COMMAND_SUBCMDS(cmds), COMMAND_DEFAULT(cmds),
                  nest + 1);
    ++cmds;
  }
}

DEFINE_COMMAND(ROOT, HELP, NULL, pObj, arg, pOut) {
  (*CLI_PTERM->printf_P)(CLI_PTERM->pObj, S("Available commands:\n"));
  cmdHelpMain(CLI_PTERM, commandArray_ROOT, 0, 0);
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_HISTORY, SHOW, NULL, pObj, arg, pOut) {
  return Editor_PrintHistory(&editor);
}

DEFINE_COMMAND(ROOT_HISTORY, CLEAR, NULL, pObj, arg, pOut) {
  Editor_ClearHistory(&editor);
  return RESULT_OK;
}

DEFINE_COMMAND_ARRAY(ROOT, HISTORY, SHOW);
