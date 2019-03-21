#include "editor.h"
#include <string.h>

static const char *defaultPrompt = "> ";

static void showPrompt(Editor *pEditor) {
  pEditor->terminal.printf_P(pEditor->terminal.pObj, S("\n%s"),
                             pEditor->prompt);
}

result_t Editor_Init(Editor *pEditor, CommandProcessor *pCmdProc,
                     Terminal *pTerminal, char buffer[], unsigned bufferSize,
                     unsigned maxLineLength) {
#ifdef ENABLE_ARGUMENT_CHECKS
  if (maxLineLength < EDITOR_LINE_LENGTH_MIN ||
      lineBufLength > EDITOR_LINE_LENGTH_MAX)
    return S("Editor: invalid maximum line length");
  if (bufferSize < 3 * maxLineLength)
    return S("Editor: buffer too small");
#endif

  /*
   *  To optimize calls by avoiding double pointer dereferencing,
   *  Editor structure contains copies of both delegate structures,
   *  instead of pointers to them.
   *
   *  Organization of data in supplied buffer:
   *  [                 0 .. maxLineLength     ]  line buffer
   *  [     maxLineLength .. 2 * maxLineLength ]  match buffer
   *  [ 2 * maxLineLength .. bufferSize        ]  history buffer
   */

  pEditor->terminal = *pTerminal;
  pEditor->cmdProc = *pCmdProc;
  pEditor->line = buffer;
  pEditor->maxLineLength = maxLineLength;
  pEditor->match = buffer + maxLineLength;
  pEditor->prompt = defaultPrompt;
  pEditor->promptLen = 2;
  ByteFifo_Init(&pEditor->history, (uint8_t *)pEditor->match + maxLineLength,
                bufferSize - 2 * maxLineLength);
  return RESULT_OK;
}

static void setCursorX(Editor *pEditor, unsigned nx) {
  unsigned x, y;
  pEditor->terminal.getCursorPosition(pEditor->terminal.pObj, &x, &y);
  pEditor->terminal.setCursorPosition(pEditor->terminal.pObj, nx, y);
}

static void refreshCurrentLine(Editor *pEditor) {
  void *pTerminal = pEditor->terminal.pObj;
  setCursorX(pEditor, 1 + pEditor->promptLen);
  pEditor->terminal.putChar(pTerminal, TERMINAL_ERASE);
  if (pEditor->historyPosition == ByteFifo_Length(&pEditor->history)) {
    pEditor->terminal.printf_P(pTerminal, S("%.*s"), pEditor->lineLen,
                               pEditor->line);
  } else {
    uint8_t ch;
    unsigned p = pEditor->historyPosition;
    while ((ch = ByteFifo_Peek(&pEditor->history, p++)) != '\0')
      pEditor->terminal.putChar(pTerminal, ch);
  }
}

typedef struct CompletionInfo_struct {
  Editor *pEditor;
  unsigned cnt;
} CompletionInfo;

static bool completionCallback(void *pArg) {
  CompletionInfo *pCi = (CompletionInfo *)pArg;
  if (++pCi->cnt == 1) {
    if (strncmp(pCi->pEditor->match, pCi->pEditor->line,
                pCi->pEditor->lineLen) != 0 ||
        pCi->pEditor->match[pCi->pEditor->lineLen] != '\0') {
      uint8_t i = -1;
      while (pCi->pEditor->match[++i])
        pCi->pEditor->line[i] = pCi->pEditor->match[i];
      pCi->pEditor->lineLen = i;
      return false;
    }
  } else {
    pCi->pEditor->terminal.printf_P(pCi->pEditor->terminal.pObj, S("\n%s"),
                                    pCi->pEditor->match);
  }
  return true;
}

result_t Editor_Run(Editor *pEditor) {
#define PUTCHAR(x) pEditor->terminal.putChar(pTerminal, (x))
#define GETCHAR(x) pEditor->terminal.getChar(pTerminal, (x))

  void *pTerminal = pEditor->terminal.pObj;
  unsigned cursorPos = 0;
  bool inHistory = false;
  uint8_t ch;

  pEditor->historyPosition = ByteFifo_Length(&pEditor->history);
  pEditor->lineLen = 0;

  showPrompt(pEditor);
  ch = GETCHAR(0);
  for (;;) {
    while (!inHistory) {
      switch (ch) {
      case TERMINAL_ERROR:
        return RESULT_OK;
      case TERMINAL_DOWN:
        break;
      case TERMINAL_UP:
        if (pEditor->historyPosition > 0)
          inHistory = true;
        break;
      case TERMINAL_RIGHT:
        if (cursorPos < pEditor->lineLen) {
          ++cursorPos;
          PUTCHAR(TERMINAL_RIGHT);
        }
        break;
      case TERMINAL_LEFT:
        if (cursorPos > 0) {
          --cursorPos;
          PUTCHAR(TERMINAL_LEFT);
        }
        break;
      case TERMINAL_HOME:
        cursorPos = 0;
        setCursorX(pEditor, 1 + pEditor->promptLen);
        break;
      case TERMINAL_END:
        cursorPos = pEditor->lineLen;
        setCursorX(pEditor, 1 + pEditor->promptLen + cursorPos);
        break;
      case TERMINAL_BS:
        if (cursorPos > 0) {
          uint8_t di;
          --cursorPos;
          --pEditor->lineLen;
          PUTCHAR(TERMINAL_LEFT);
          PUTCHAR(TERMINAL_SAVE);
          for (di = cursorPos; di < pEditor->lineLen; ++di) {
            pEditor->line[di] = pEditor->line[di + 1];
            PUTCHAR(pEditor->line[di]);
          }
          PUTCHAR(TERMINAL_ERASE);
          PUTCHAR(TERMINAL_RESTORE);
        }
        break;
      case TERMINAL_HINT: {
        CompletionInfo ci;
        ci.pEditor = pEditor;
        ci.cnt = 0;
        pEditor->line[pEditor->lineLen] = '\0';
        pEditor->cmdProc.complete(pEditor->cmdProc.pObj, pEditor->line,
                                  completionCallback, &ci,
                                  pEditor->maxLineLength, pEditor->match);
        if (ci.cnt >= 1) {
          if (ci.cnt > 1)
            showPrompt(pEditor);
          else
            setCursorX(pEditor, 1 + pEditor->promptLen);
          pEditor->terminal.printf_P(pTerminal, S("%.*s"), pEditor->lineLen,
                                     pEditor->line);
        }
        cursorPos = pEditor->lineLen;
        break;
      }
      case TERMINAL_NEWLINE:
        pEditor->terminal.putChar(pTerminal, TERMINAL_NEWLINE);
        pEditor->line[pEditor->lineLen] = '\0';
        // remove oldest lines if there's no room for current line in history
        while (pEditor->history.capacity - ByteFifo_Length(&pEditor->history) <=
               pEditor->lineLen)
          while (ByteFifo_Get(&pEditor->history)) {
          }
        // add current line (if not empty) to history
        if (pEditor->lineLen > 0) {
          ByteFifo_Write(&pEditor->history, pEditor->line,
                         pEditor->lineLen + 1);
          pEditor->historyPosition = ByteFifo_Length(&pEditor->history);
        }
        // execute
        result_t res = pEditor->cmdProc.execute(
            pEditor->cmdProc.pObj, pEditor->line, &pEditor->terminal);
        if (res != RESULT_OK)
          pEditor->terminal.printf_P(pTerminal, S("ERROR: %S"), res);
        showPrompt(pEditor);
        cursorPos = 0;
        pEditor->lineLen = 0;
        break;
      default:
        if (pEditor->lineLen < pEditor->maxLineLength - 1) {
          if (pEditor->lineLen == cursorPos) {
            // add character at the end
            pEditor->line[cursorPos] = ch;
            pEditor->terminal.putChar(pTerminal, ch);
            ++pEditor->lineLen;
            ++cursorPos;
          } else {
            // or insert somewhere in the middle
            uint8_t di;
            for (di = pEditor->lineLen; di > cursorPos; --di) {
              pEditor->line[di] = pEditor->line[di - 1];
            }
            pEditor->line[cursorPos] = ch;
            pEditor->terminal.putChar(pTerminal, ch);
            ++pEditor->lineLen;
            ++cursorPos;
            pEditor->terminal.putChar(pTerminal, TERMINAL_SAVE);
            for (di = cursorPos; di < pEditor->lineLen; ++di) {
              pEditor->terminal.putChar(pTerminal, pEditor->line[di]);
            }
            pEditor->terminal.putChar(pTerminal, TERMINAL_RESTORE);
          }
        }
      }
      if (!inHistory)
        ch = GETCHAR(0);
    }
    while (inHistory) {
      switch (ch) {
      case TERMINAL_ERROR:
        return RESULT_OK;
      case TERMINAL_UP:
        if (pEditor->historyPosition > 0) {
          --pEditor->historyPosition;
          cursorPos = 0;
          while (pEditor->historyPosition > 0) {
            --pEditor->historyPosition;
            ++cursorPos;
            if (!(ByteFifo_Peek(&pEditor->history, pEditor->historyPosition))) {
              ++pEditor->historyPosition;
              --cursorPos;
              break;
            }
          }
          refreshCurrentLine(pEditor);
        }
        break;
      case TERMINAL_DOWN: {
        unsigned len = ByteFifo_Length(&pEditor->history);
        cursorPos = 0;
        if (pEditor->historyPosition < len) {
          while (ByteFifo_Peek(&pEditor->history,
                               pEditor->historyPosition + cursorPos) != '\0')
            ++cursorPos;
        }
        pEditor->historyPosition += cursorPos + 1;
        refreshCurrentLine(pEditor);
        if (pEditor->historyPosition == len) {
          inHistory = false;
          cursorPos = pEditor->lineLen;
        }
        break;
      }
      case TERMINAL_RIGHT:
        if (ByteFifo_Peek(&pEditor->history,
                          pEditor->historyPosition + cursorPos) != '\0') {
          ++cursorPos;
          pEditor->terminal.putChar(pTerminal, TERMINAL_RIGHT);
        }
        break;
      case TERMINAL_LEFT:
        if (cursorPos > 0) {
          --cursorPos;
          pEditor->terminal.putChar(pTerminal, TERMINAL_LEFT);
        }
        break;
      case TERMINAL_HOME:
        cursorPos = 0;
        setCursorX(pEditor, 1 + pEditor->promptLen);
        break;
      case TERMINAL_END:
        while (ByteFifo_Peek(&pEditor->history,
                             pEditor->historyPosition + cursorPos) != '\0')
          ++cursorPos;
        setCursorX(pEditor, 1 + pEditor->promptLen + cursorPos);
        break;
      default: {
        // copy line from history to current buffer
        // and let the other loop take care of the rest
        unsigned k = 0;
        uint8_t c;
        while (
            (c = ByteFifo_Peek(&pEditor->history, pEditor->historyPosition++)))
          pEditor->line[k++] = c;
        pEditor->lineLen = k;
        pEditor->historyPosition = ByteFifo_Length(&pEditor->history);
        inHistory = false;
      }
      }
      if (inHistory)
        ch = GETCHAR(0);
    }
  }
#undef PUTCHAR
#undef GETCHAR
}

result_t Editor_PrintHistory(Editor *pEditor) {
  unsigned l = ByteFifo_Length(&pEditor->history);
  unsigned k;
  for (k = 0; k < l; ++k) {
    uint8_t c = ByteFifo_Peek(&pEditor->history, k);
    (*pEditor->terminal.putChar)(pEditor->terminal.pObj, c ? c : '\n');
  }
  return (*pEditor->terminal.printf_P)(pEditor->terminal.pObj,
                                       S("Uses %d of %d bytes\n"), l,
                                       pEditor->history.capacity);
}

void Editor_ClearHistory(Editor *pEditor) { ByteFifo_Clear(&pEditor->history); }

#ifdef UNITTEST

#include "printf.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct TestTerminal_struct {
  const char *pIn;
  char *pOut;
} TestTerminal;

static uint8_t testConGetChar(void *pObj, uint16_t timeout) {
  TestTerminal *pCon = (TestTerminal *)pObj;
  uint8_t c;

  if ((c = *pCon->pIn) != '\0')
    ++pCon->pIn;
  return c;
}

static void testConPutChar(void *pObj, char c) {
  TestTerminal *pCon = (TestTerminal *)pObj;
  *pCon->pOut++ = c;
}

static result_t testConPrintf_P(void *pObj, immutable_str fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  kvprintf_P(fmt, &testConPutChar, pObj, ap);
  va_end(ap);
  return RESULT_OK;
}

static result_t testCPComplete(void *pObj, const char *str,
                               bool (*callback)(void *pArg), void *pArg,
                               uint8_t matchLen, char *match) {
  return RESULT_OK;
}

static result_t testCPExecute(void *pObj, const char *str, void *pOut) {
  (void)pOut;
  testConPrintf_P(pObj, "exec: {%s}", str);
  return RESULT_OK;
}

void testEditor(void) {
  TestTerminal tc;
  Terminal con;
  CommandProcessor cp;
  Editor le;
  static char inbuf[1024];
  static char outbuf[1024];
  static char edbuf[5000];

  tc.pIn = inbuf;
  tc.pOut = outbuf;
  con.getChar = &testConGetChar;
  con.putChar = (void (*)(void *, uint8_t)) & testConPutChar;
  con.printf_P = &testConPrintf_P;
  con.pObj = &tc;
  cp.execute = &testCPExecute;
  cp.complete = &testCPComplete;
  cp.pObj = &tc;

  assert(RESULT_OK == Editor_Init(&le, &cp, &con, edbuf, 5000, 77));

  strcpy(inbuf, "abc\r");
  strcat(inbuf, "\x1b[A");
  assert(RESULT_OK == Editor_Run(&le));

  printf("{%s}\n", outbuf);

  printf("Editor tests passed\n\n");
}

#endif // UNITTEST
