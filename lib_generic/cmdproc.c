#include <ctype.h>

#include "astring.h"
#include "cmdarg.h"
#include "cmdproc.h"

/** \brief Parse identifier in form of [A-Za-z][A-Za-z0-9]*
 *
 *  \param[in]  str  String to parse.
 *  \param[out] len  Position of first character in string past the
 *                   parsed identifier.
 *
 *  \return     RESULT_OK or error message.
 *
 *  \note Upon return, len is always valid, i.e. in case first character
 *        is not a letter len == 0.
 */
static result_t parseIdent(const char *str, uint8_t *len) {
  unsigned char c = *str;
  *len = 0;
  if (!c)
    return S("CMDPROC: expected identifier");
  if (!isalpha(c))
    return S("CMDPROC: expected identifier");
  while (isalnum((unsigned char)str[++*len])) {
  }
  return RESULT_OK;
}

/** \brief Parse separator ('.' or ':')
 *
 *  \param[in]  str   String to parse.
 *  \param[out] len   Position of first character in string past the
 *                    parsed separator.
 *
 *  \return     RESULT_OK or error message.
 */
static result_t parseSeparator(const char *str, uint8_t *len) {
  unsigned char c = *str;
  *len = 0;
  if (c != '.' && c != ':')
    return S("CMDPROC: expected separator");
  ++*len;
  return RESULT_OK;
}

/** \brief Search for command matching given identifier.
 *
 *  \param[in]  cmds    Command array to search in.
 *  \param[in]  str     Identifier. Does not need to be NUL-terminated,
 *                      its length is passed in len.
 *  \param[in]  len     Length of identifier.
 */
static Command *findCommand(Command cmds[], const char *str, uint8_t len) {
  immutable_str name;
  while (!!(name = COMMAND_NAME(cmds))) {
    uint8_t i;
    for (i = 0; i < len; ++i) {
      if (READ_IMMUTABLE_BYTE(&name[i]) != toupper((unsigned char)str[i]))
        goto nextCmd;
    }
    if (!READ_IMMUTABLE_BYTE(&name[i]))
      return cmds;
  nextCmd:
    ++cmds;
  }
  return NULL;
}

result_t CmdProc_Complete(void *pObj, const char *str,
                          bool (*callback)(void *pArg), void *pArg,
                          uint8_t matchLen, char *match) {
#define PCHAR(_c)                                                              \
  do {                                                                         \
    if (matchIx >= matchLen)                                                   \
      return S("CMDPROC: buffer overflow");                                    \
    match[matchIx] = _c;                                                       \
    matchIx++;                                                                 \
  } while (0)

  Command *cmds = ((Command *)pObj);
  uint8_t matchIx = 0;
  uint8_t c;

  // canonicalize full command path into match buffer
  for (;;) {
    uint8_t len;
    str = skipSpaces(str);
    result_t res = parseIdent(str, &len);
    if (len == 0)
      break;
    while (len--)
      PCHAR(toupper((uint8_t)(*str++)));
    str = skipSpaces(str);
    res = parseSeparator(str, &len);
    if (res != RESULT_OK)
      break;
    PCHAR('.');
    ++str;
  }
  // extra characters on line, won't match anything
  if (*str)
    return RESULT_OK;

  unsigned lbegin = 0;
  unsigned lend;
  immutable_str name;
  bool wantExact;
  bool foundExact;
  Command *pCmd;
  Command *pFound;
  unsigned matches;

  match[matchIx] = '\0';
  matchIx = 0;
  for (;;) {
    // count matching idents at current level and find longest unambiguously
    // matching string if there's only one and it's a leaf - return match if
    // there's only one and it's not a leaf - list matches in deeper level if
    // there's more than one - list all, with ' ' on leaves and '.' on internal
    // nodes
    matches = 0;
    pFound = NULL;
    pCmd = cmds;
    lend = lbegin;
    while ((c = match[lend]) && c != '.')
      ++lend;
    wantExact = !!c; // true when c=='.', false when c=='\0'
    foundExact = false;
    // currently matched ident is match[lbegin .. lend]
    while ((name = COMMAND_NAME(pCmd))) {
      //			if (0 == (wantExact ? strncmp_P(match + lbegin,
      //name, lend - lbegin) : strncmp_P(match + lbegin, name, lend - lbegin)))
      if (0 == strncmp_P(match + lbegin, name, lend - lbegin)) {
        if (READ_IMMUTABLE_BYTE(name + (lend - lbegin)) == 0)
          foundExact = true;
        ++matches;
        if (!pFound) {
          pFound = pCmd;
          if (!wantExact) {
            matchIx = lbegin;
            while ((c = READ_IMMUTABLE_BYTE(name++)))
              PCHAR(c);
          } else {
            matchIx = lend;
            break;
          }
        } else {
          // cut to longest unambiguous string
          unsigned k = lbegin;
          while (k < matchIx && match[k] == READ_IMMUTABLE_BYTE(name)) {
            ++k;
            ++name;
          }
          matchIx = k;
        }
        if (foundExact)
          break;
      }
      ++pCmd;
    }
    if (matches == 0)
      return RESULT_OK;
    if (matches == 1) {
      if (COMMAND_ISLEAF(pFound)) {
        if (wantExact)
          return RESULT_OK; // input string contained dot after leaf node ident
        PCHAR(' ');
        PCHAR('\0');
        (*callback)(pArg);
        return RESULT_OK;
      } else {
        if (wantExact)
          ++matchIx;
        else {
          if (!foundExact && COMMAND_DEFAULT(pFound)) {
            PCHAR('\0');
            (*callback)(pArg);
            return RESULT_OK;
          }
          PCHAR('.');
          match[matchIx] = '\0';
        }
        cmds = COMMAND_SUBCMDS(pFound);
        lbegin = matchIx;
        continue;
      }
    }
    //		assert(!exact);
    // unambiguous completion first
    PCHAR('\0');
    if (!(*callback)(pArg))
      return RESULT_OK;
    pCmd = cmds;
    // then all alternatives available at this level
    while ((name = COMMAND_NAME(pCmd))) {
      if (0 == strncmp_P(match + lbegin, name, lend - lbegin)) {
        matchIx = lbegin;
        while ((c = READ_IMMUTABLE_BYTE(name++)))
          PCHAR(c);
        if (!COMMAND_ISLEAF(pCmd))
          PCHAR('.');
        PCHAR('\0');
        if (!(*callback)(pArg))
          return RESULT_OK;
      }
      ++pCmd;
    }
    return RESULT_OK;
  }
#undef PCHAR
}

result_t CmdProc_Execute(void *pObj, const char *str, void *pOut) {
  Command *cmds = ((Command *)pObj);
  Command *pDefCmd = (Command *)0;
  result_t result;
  uint8_t len;

  // ignore comments
  str = skipSpaces(str);
  if (*str == '#')
    return RESULT_OK;

  while (cmds) {
    str = skipSpaces(str);
    result = parseIdent(str, &len);
    if (result != RESULT_OK)
      return result;

    Command *pCmd = findCommand(cmds, str, len);
    if (pCmd) {
      str += len;
      result = parseSeparator(str, &len);
      str += len;
      if (result == RESULT_OK) {
        if (COMMAND_ISLEAF(pCmd))
          return S("CMDPROC: unexpected separator");
        cmds = COMMAND_SUBCMDS(pCmd);
        pDefCmd = COMMAND_DEFAULT(pCmd);
      } else {
        while (!COMMAND_ISLEAF(pCmd)) {
          pCmd = COMMAND_DEFAULT(pCmd);
          if (!pCmd)
            return result;
        }
        return (*(COMMAND_FUNC(pCmd)))(COMMAND_POBJ(pCmd), str, pOut);
      }
    } else {
      if (!pDefCmd || COMMAND_ISLEAF(pDefCmd))
        return S("CMDPROC: unknown command");
      cmds = COMMAND_SUBCMDS(pDefCmd);
      pDefCmd = COMMAND_DEFAULT(pDefCmd);
    }
  }
  return S("CMDPROC: panic");
}

result_t CmdProc_Init(CommandProcessor *pCmdProc, Command commands[]) {
  pCmdProc->pObj = (void *)commands;
  pCmdProc->complete = &CmdProc_Complete;
  pCmdProc->execute = &CmdProc_Execute;
  return RESULT_OK;
}

#ifdef UNITTEST

#include <assert.h>
#include <stdio.h>
#include <string.h>
//#define printf(...)

#define OUTPUT_LEN 1024

static char output[OUTPUT_LEN];

static result_t execute(void *pObj, const char *str, void *pOut) {
  (void)pOut;
  snprintf(output, OUTPUT_LEN, "executed for %08X with str=|%s|",
           *((int *)pObj), str);
  (*((int *)pObj))++;
  return RESULT_OK;
}

/*
 * the commands below build the following tree:
 * ~LEAF
 * INTERNAL
 *   ~[DEFAULT]
 *   ~OTHER
 * INTERNAL2
 *   [DEFAULT]
 *     ~[VALUE]
 *     ~UNIT
 * INTERNAL3
 *   [DEFAULT]
 *     ~VALUE
 *
 * [] - optional
 * ~  - leaf node
 */
COMMAND_ARRAY_DECLARE(testArray);

IMMUTABLE_STR(leafName) = "LEAF";

static int leafInt = 0xDEADBEEF;

Command leafEntry COMMAND_IN_ARRAY(testArray) = {
    .name = leafName, .isLeaf = true, .body.command = {&execute, &leafInt}};

IMMUTABLE_STR(internalName) = "INTERNAL";
COMMAND_ARRAY_DECLARE(subArray);
Command defaultEntry COMMAND_IN_ARRAY(subArray);
Command internalEntry COMMAND_IN_ARRAY(testArray) = {
    .name = internalName,
    .isLeaf = false,
    .body.subcommands = {subArray, &defaultEntry}};

IMMUTABLE_STR(defaultName) = "DEFAULT";
static int defaultInt = 0xBAADF00D;
Command defaultEntry COMMAND_IN_ARRAY(subArray) = {
    .name = defaultName,
    .isLeaf = true,
    .body.command = {&execute, &defaultInt}};

IMMUTABLE_STR(otherName) = "OTHER";
static int otherInt = 0xFEE1DEAD;
Command otherEntry COMMAND_IN_ARRAY(subArray) = {
    .name = otherName, .isLeaf = true, .body.command = {&execute, &otherInt}};

IMMUTABLE_STR(internal2Name) = "INTERNAL2";
COMMAND_ARRAY_DECLARE(subArray2);
Command defaultEntry2 COMMAND_IN_ARRAY(subArray2);
Command internal2Entry COMMAND_IN_ARRAY(testArray) = {
    .name = internal2Name,
    .isLeaf = false,
    .body.subcommands = {subArray2, &defaultEntry2}};

COMMAND_ARRAY_DECLARE(defaultArray);
Command valueEntry COMMAND_IN_ARRAY(defaultArray);
Command defaultEntry2 COMMAND_IN_ARRAY(subArray2) = {
    .name = defaultName,
    .isLeaf = false,
    .body.subcommands = {defaultArray, &valueEntry}};

IMMUTABLE_STR(valueName) = "VALUE";
static int valueInt = 0xBADCAB1E;
Command valueEntry COMMAND_IN_ARRAY(defaultArray) = {
    .name = valueName, .isLeaf = true, .body.command = {&execute, &valueInt}};

IMMUTABLE_STR(unitName) = "UNIT";
static int unitInt = 0xC01DCAFE;
Command unitEntry COMMAND_IN_ARRAY(defaultArray) = {
    .name = unitName, .isLeaf = true, .body.command = {&execute, &unitInt}};

IMMUTABLE_STR(internal3Name) = "INTERNAL3";
COMMAND_ARRAY_DECLARE(subArray4);
Command defaultEntry4 COMMAND_IN_ARRAY(subArray4);
Command internal3Entry COMMAND_IN_ARRAY(testArray) = {
    .name = internal3Name,
    .isLeaf = false,
    .body.subcommands = {subArray4, &defaultEntry4}};

COMMAND_ARRAY_DECLARE(defaultArray2);
Command specialEntry COMMAND_IN_ARRAY(defaultArray2);
Command defaultEntry4 COMMAND_IN_ARRAY(subArray4) = {
    .name = defaultName,
    .isLeaf = false,
    .body.subcommands = {defaultArray2, 0}};

IMMUTABLE_STR(specialName) = "SPECIAL";
static int specialInt = 0xA1115BAD;
Command specialEntry COMMAND_IN_ARRAY(defaultArray2) = {
    .name = specialName,
    .isLeaf = true,
    .body.command = {&execute, &specialInt}};

// tests begin here

static void testParseIdent(void) {
  result_t result;
  const char *str;
  uint8_t len;

  str = "";
  result = parseIdent(str, &len);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  str = "  ";
  result = parseIdent(str, &len);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  str = "0Ident";
  result = parseIdent(str, &len);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  str = "Ident0";
  result = parseIdent(str, &len);
  assert(result == RESULT_OK);
  assert(len == 6);

  str = "Ident0?";
  result = parseIdent(str, &len);
  assert(result == RESULT_OK);
  assert(len == 6);

  printf("testParseIdent passed\n\n");
}

static void testParseSeparator(void) {
  result_t result;
  const char *str;
  uint8_t len;

  str = "";
  result = parseSeparator(str, &len);
  assert(0 == strcmp(result, "CMDPROC: expected separator"));

  str = " .";
  result = parseSeparator(str, &len);
  assert(0 == strcmp(result, "CMDPROC: expected separator"));

  str = "::..";
  result = parseSeparator(str, &len);
  assert(result == RESULT_OK);
  assert(len == 1);

  str = "....";
  result = parseSeparator(str, &len);
  assert(result == RESULT_OK);
  assert(len == 1);

  printf("testParseSeparator passed\n\n");
}

static void testExecute(void) {
  static CommandProcessor cmdProc;
  void *pObj;
  result_t result;

  CmdProc_Init(&cmdProc, testArray);
  pObj = cmdProc.pObj;

  *output = '\0';
  result = CmdProc_Execute(pObj, "", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "  ", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "   ^", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "   0", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "   a0^", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unknown command"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "   notexistent0 command", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unknown command"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "LEAF.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unexpected separator"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "leaf:", output);
  assert(0 == strcmp(result, "CMDPROC: unexpected separator"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "LEAfZ", output);
  assert(0 == strcmp(result, "CMDPROC: unknown command"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "leaf", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for DEADBEEF with str=||"));
  assert(leafInt == 0xDEADBEF0);

  *output = '\0';
  result = CmdProc_Execute(pObj, "lEaF some arg", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for DEADBEF0 with str=| some arg|"));
  assert(leafInt == 0xDEADBEF1);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for BAADF00D with str=||"));
  assert(defaultInt == 0xBAADF00E);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL   axd", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for BAADF00E with str=|   axd|"));
  assert(defaultInt == 0xBAADF00F);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL.DEFAULT ", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for BAADF00F with str=| |"));
  assert(defaultInt == 0xBAADF010);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL.DEFAULTX", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unknown command"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL.OTHER?", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for FEE1DEAD with str=|?|"));
  assert(otherInt == 0xFEE1DEAE);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2????", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for BADCAB1E with str=|????|"));
  assert(valueInt == 0xBADCAB1F);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.DEFAULT-??", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for BADCAB1F with str=|-??|"));
  assert(valueInt == 0xBADCAB20);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.DEFAULT.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected identifier"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.DEFAULT.VALUE*", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for BADCAB20 with str=|*|"));
  assert(valueInt == 0xBADCAB21);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.VALUE.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unexpected separator"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.DEFAULT.VALUE.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unexpected separator"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.DEFAULT.UNIT*", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for C01DCAFE with str=|*|"));
  assert(unitInt == 0xC01DCAFF);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL2.UNIT*", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for C01DCAFF with str=|*|"));
  assert(unitInt == 0xC01DCB00);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL3", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: expected separator"));

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL3.DEFAULT.SPECIAL*", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for A1115BAD with str=|*|"));
  assert(specialInt == 0xA1115BAE);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL3.SPECIAL*", output);
  printf("{%s} %s\n", result, output);
  assert(result == RESULT_OK);
  assert(0 == strcmp(output, "executed for A1115BAE with str=|*|"));
  assert(specialInt == 0xA1115BAF);

  *output = '\0';
  result = CmdProc_Execute(pObj, "INTERNAL3.SPECIAL.", output);
  printf("{%s} %s\n", result, output);
  assert(0 == strcmp(result, "CMDPROC: unexpected separator"));

  printf("testExecute passed\n\n");
}

typedef struct CompletionInfo_struct {
  char *match;
  unsigned cnt;
  unsigned limit;
} CompletionInfo;

static bool testCompleteCallback(void *pArg) {
  CompletionInfo *pCi = (CompletionInfo *)pArg;
  pCi->cnt++;
  strcat(output, "|");
  strcat(output, pCi->match);
  printf("{%s}\n", pCi->match);
  return pCi->cnt < pCi->limit;
}

static void initInfo(CompletionInfo *ci, char *match, unsigned limit) {
  *output = '\0';
  ci->match = match;
  ci->cnt = 0;
  ci->limit = limit;
}

static void testComplete(void) {
  static CommandProcessor cmdProc;
  static CompletionInfo ci;
  static char match[230];
  result_t res;
  void *pObj;

  CmdProc_Init(&cmdProc, testArray);
  pObj = cmdProc.pObj;

  initInfo(&ci, match, 100);
  res = CmdProc_Complete(pObj, "", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "||LEAF|INTERNAL.|INTERNAL2.|INTERNAL3."));
  assert(5 == ci.cnt);

  initInfo(&ci, match, 1);
  res = CmdProc_Complete(pObj, "", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "|"));
  assert(1 == ci.cnt);

  initInfo(&ci, match, 2);
  res = CmdProc_Complete(pObj, "", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "||LEAF"));
  assert(2 == ci.cnt);

  initInfo(&ci, match, 3);
  res = CmdProc_Complete(pObj, "", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "||LEAF|INTERNAL."));
  assert(3 == ci.cnt);

  initInfo(&ci, match, 4);
  res = CmdProc_Complete(pObj, "", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "||LEAF|INTERNAL.|INTERNAL2."));
  assert(4 == ci.cnt);

  initInfo(&ci, match, 5);
  res = CmdProc_Complete(pObj, "", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "||LEAF|INTERNAL.|INTERNAL2.|INTERNAL3."));
  assert(5 == ci.cnt);

  initInfo(&ci, match, 100);
  res = CmdProc_Complete(pObj, "  test.azbest  ", testCompleteCallback, &ci,
                         230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, ""));
  assert(0 == ci.cnt);

  initInfo(&ci, match, 100);
  res =
      CmdProc_Complete(pObj, "  leaf ", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "|LEAF "));
  assert(1 == ci.cnt);

  initInfo(&ci, match, 100);
  res =
      CmdProc_Complete(pObj, "  leaf. ", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, ""));
  assert(0 == ci.cnt);

  initInfo(&ci, match, 100);
  res = CmdProc_Complete(pObj, "  lea ", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "|LEAF "));
  assert(1 == ci.cnt);

  initInfo(&ci, match, 100);
  res =
      CmdProc_Complete(pObj, "  inte ", testCompleteCallback, &ci, 230, match);
  printf("{%s}\n", output);
  assert(res == RESULT_OK);
  assert(0 == strcmp(output, "|INTERNAL|INTERNAL.|INTERNAL2.|INTERNAL3."));
  assert(4 == ci.cnt);

  /*	initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal   ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output,
     "|INTERNAL|INTERNAL.|INTERNAL2.|INTERNAL3.")); assert(4 == ci.cnt);

          initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal2 ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output,
     "|INTERNAL2.DEFAULT.|INTERNAL2.DEFAULT.VALUE |INTERNAL2.DEFAULT.UNIT "));
          assert(3 == ci.cnt);

          initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal. ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output, "|INTERNAL.|INTERNAL.DEFAULT
     |INTERNAL.OTHER ")); assert(3 == ci.cnt);

          initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal.d ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output, "|INTERNAL.DEFAULT "));
          assert(1 == ci.cnt);

          initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal.o ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output, "|INTERNAL.OTHER "));
          assert(1 == ci.cnt);

          initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal.k ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output, ""));
          assert(0 == ci.cnt);

          initInfo(&ci, match, 100);
          res = CmdProc_Complete(pObj, "  internal2. ", testCompleteCallback,
     &ci, 230, match); printf("{%s}\n", output); assert(res == RESULT_OK);
          assert(0 == strcmp(output,
     "|INTERNAL2.DEFAULT.|INTERNAL2.DEFAULT.VALUE |INTERNAL2.DEFAULT.UNIT "));
          assert(3 == ci.cnt);*/

  printf("testComplete passed\n\n");
}

void testCmdProc(void) {
  testParseIdent();
  testParseSeparator();
  testExecute();
  testComplete();

  printf("CmdProc tests passed\n\n");
}

#undef OUTPUT_LEN

#endif // UNITTEST
