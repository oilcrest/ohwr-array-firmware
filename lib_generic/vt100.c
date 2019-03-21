#include "vt100.h"

#define PSTREAM ((Stream *)pObj)

uint8_t VT100_GetChar(void *pObj, uint16_t timeout);
void VT100_PutChar(void *pObj, uint8_t c);
result_t VT100_GetCursorPosition(void *pObj, unsigned *pX, unsigned *pY);
result_t VT100_SetCursorPosition(void *pObj, unsigned x, unsigned y);

void VT100_Init(Terminal *pTerminal, Stream *pStream) {
  pTerminal->pObj = (void *)pStream;
  pTerminal->getChar = &VT100_GetChar;
  pTerminal->putChar = &VT100_PutChar;
  // Stream_Printf_P uses Stream * in place of void *
  pTerminal->printf_P =
      (result_t(*)(void *, immutable_str, ...)) & Stream_Printf_P;
  pTerminal->getCursorPosition = &VT100_GetCursorPosition;
  pTerminal->setCursorPosition = &VT100_SetCursorPosition;
}

static uint8_t getRaw(void *pObj, uint16_t timeout) {
  uint8_t c;
  Stream_Read(PSTREAM, &c, 1, NULL);
  return c;
}

#define VT100_CURSOR_POSITION 0xFF

static uint8_t getCharWithParams(void *pObj, uint16_t timeout, unsigned *pParm1,
                                 unsigned *pParm2) {
  uint8_t b;

  b = getRaw(pObj, timeout);
  switch (b) {
  case 0x20 ... 0x7E:
    return b;
  case 0x7f:
  case 0x08:
    return TERMINAL_BS;
  case 0x09:
    return TERMINAL_HINT;
  case 0x0D:
    return TERMINAL_NEWLINE;
  case 0x00:
    return TERMINAL_ERROR;
  case 0x1B:
    b = getRaw(pObj, timeout);
    if (b == '[') {
      *pParm1 = 0;
      *pParm2 = 0;
      for (;;) {
        b = getRaw(pObj, timeout);
        if (b >= '0' && b <= '9')
          *pParm1 = *pParm1 * 10 + (b - '0');
        else if (b == ';') {
          *pParm2 = 0;
          for (;;) {
            b = getRaw(pObj, timeout);
            if (b >= '0' && b <= '9')
              *pParm2 = *pParm2 * 10 + (b - '0');
            else if (b == ';') {
            } else
              goto doCmd;
          }
          break;
        } else
          break;
      }
    doCmd:
      switch (b) {
      case 'A':
        return TERMINAL_UP;
      case 'B':
        return TERMINAL_DOWN;
      case 'C':
        return TERMINAL_RIGHT;
      case 'D':
        return TERMINAL_LEFT;
      case 'R':
        return VT100_CURSOR_POSITION;
      case '~':
        if (*pParm1 == 1)
          return TERMINAL_HOME;
        else if (*pParm1 == 4)
          return TERMINAL_END;
      }
      break;
    }
  }
  return ' ';
}

uint8_t VT100_GetChar(void *pObj, uint16_t timeout) {
  unsigned parm1;
  unsigned parm2;
  uint8_t p = getCharWithParams(pObj, timeout, &parm1, &parm2);
  return p;
}

void VT100_PutChar(void *pObj, uint8_t c) {
  switch (c) {
  case 0x20 ... 0xFF:
    Stream_Write(PSTREAM, &c, 1, NULL);
    break;
  case TERMINAL_SAVE:
    Stream_Printf_P(PSTREAM, S("\x1b\x37"));
    break;
  case TERMINAL_RESTORE:
    Stream_Printf_P(PSTREAM, S("\x1b\x38"));
    break;
  case TERMINAL_ERASE:
    Stream_Printf_P(PSTREAM, S("\x1b[0K"));
    break;
  case TERMINAL_LEFT:
    Stream_Printf_P(PSTREAM, S("\x1b[D"));
    break;
  case TERMINAL_RIGHT:
    Stream_Printf_P(PSTREAM, S("\x1b[C"));
    break;
  case TERMINAL_NEWLINE:
  case '\n':
    Stream_Printf_P(PSTREAM, S("\n"));
    break;
  default: {}
  }
}

result_t VT100_GetCursorPosition(void *pObj, unsigned *pX, unsigned *pY) {
  /*
   *  Cursor position query:       ESC [ 6 n
   *  Cursor position response:    ESC [ Pl ; Pc R
   */

  Stream_Printf_P(PSTREAM, S("\x1b[6n"));

  if (getCharWithParams(pObj, 1000, pY, pX) != VT100_CURSOR_POSITION) {
    *pX = 0;
    *pY = 0;
    return S("VT100_GetCursorPosition: Failed");
  }
  return RESULT_OK;
}

result_t VT100_SetCursorPosition(void *pObj, unsigned x, unsigned y) {
  return Stream_Printf_P(PSTREAM, S("\x1b[%d;%df"), y, x);
}
