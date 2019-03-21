#include "cmdarg.h"

#include "printf.h"

#include <ctype.h>
#include <stdlib.h>

const char *skipSpaces(const char *str) {
  while (isspace((unsigned char)*str))
    ++str;
  return str;
}

result_t parseInt(const char **pStr, int32_t min, int32_t max, int32_t *val) {
  const char *str = *pStr;
  uint8_t base = 10;
  bool neg = false;
  int64_t value = 0;
  char c;

  while (isspace((unsigned char)(c = *str++))) {
  }

reswitch:
  switch (c) {
  case '-':
    neg = true;
  case '+':
    c = *str++;
    goto reswitch;
    break;
  case '0':
    c = *str++;
    if (c == 'x') {
      c = *str++;
      base = 16;
    }
    break;
  case '1' ... '9':
    break;
  default:
    return S("CMDARG: expected integer");
  }

  for (;;) {
    if (isdigit((unsigned char)c)) {
      value = value * base + (c - '0');
      if (value > (neg ? 0x80000000LL : 0x7FFFFFFFLL))
        return S("CMDARG: value out of range");
    } else if (base == 16 && isxdigit((unsigned char)c)) {
      value = value * 16 + (toupper((unsigned char)c) - ('A' - 10));
      if (value > (neg ? 0x80000000LL : 0x7FFFFFFFLL))
        return S("CMDARG: value out of range");
    } else {
      if (neg)
        value = -value;
      if (value < min || value > max)
        return S("CMDARG: value out of range");
      *val = value;
      *pStr = str - 1;
      return RESULT_OK;
    }
    c = *str++;
  }

  return RESULT_OK;
}

result_t parseFloat(const char **pStr, float *val) {
  const char *str = *pStr;

  while (isspace((unsigned char)(*str)))
    ++str;

  *val = atof(str);

  while (*str && !isspace((unsigned char)(*str)))
    ++str;

  *pStr = str;

  return RESULT_OK;
}

result_t parseOnOff(const char **pStr, bool *val) {
  char buf[4];
  result_t res;

  res = parseString(pStr, 4, buf);
  if (res != RESULT_OK)
    return res;

  if (toupper((unsigned char)buf[0]) == 'O') {
    if (toupper((unsigned char)buf[1]) == 'F' &&
        toupper((unsigned char)buf[2]) == 'F' && !buf[3]) {
      *val = false;
      return RESULT_OK;
    }
    if (toupper((unsigned char)buf[1]) == 'N' && !buf[2]) {
      *val = true;
      return RESULT_OK;
    }
  }

  return S("CMDARG: expected ON or OFF");
}

result_t parseString(const char **pStr, uint16_t len, char *val) {
#define PCHAR(_p)                                                              \
  do {                                                                         \
    if (len == 0)                                                              \
      return S("CMDARG: buffer overflow when parsing string");                 \
    *val++ = _p;                                                               \
    --len;                                                                     \
  } while (0)

  const char *str = *pStr;
  char c;

  while (isspace((unsigned char)(c = *str++))) {
  }

  // read quoted string if starts with '"'
  if (c == '"') {
    while ((c = *str++) != '\0') {
      if (c == '"') {
        PCHAR(0);
        *pStr = str;
        return RESULT_OK;
      } else if (c == '\\') {
        PCHAR(*str++);
      } else {
        PCHAR(c);
      }
    }
    return S("CMDARG: missing ending quote");
  }

  // otherwise the string is not quoted and stops on first whitespace character
  for (;;) {
    if (!c) {
      PCHAR(0);
      *pStr = str - 1;
      return RESULT_OK;
    } else if (isspace((unsigned char)c)) {
      PCHAR(0);
      *pStr = str;
      return RESULT_OK;
    }
    PCHAR(c);
    c = *str++;
  }

#undef PCHAR
}

#ifdef UNITTEST

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void testParseInt(void) {
  const char *str;
  result_t result;
  int32_t val;

  str = "  392310ABC";
  printf("|%s| => ", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  printf("|%d|%s|\n", val, str);
  assert(result == RESULT_OK);
  assert(val == 392310);
  assert(0 == strcmp(str, "ABC"));

  str = "  0";
  printf("|%s| => ", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  printf("|%d|%s|\n", val, str);
  assert(result == RESULT_OK);
  assert(val == 0);
  assert(0 == strcmp(str, ""));

  str = "  +123456789abc";
  printf("|%s| => ", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  printf("|%d|%s|\n", val, str);
  assert(result == RESULT_OK);
  assert(val == 123456789);
  assert(0 == strcmp(str, "abc"));

  str = "  -987654321abc";
  printf("|%s| => ", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  printf("|%d|%s|\n", val, str);
  assert(result == RESULT_OK);
  assert(val == -987654321);
  assert(0 == strcmp(str, "abc"));

  str = "  0x7FFFFFFFgh";
  printf("|%s| => ", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  printf("|%d|%s|\n", val, str);
  assert(result == RESULT_OK);
  assert(val == INT32_MAX);
  assert(0 == strcmp(str, "gh"));

  str = "  -0x80000000gh";
  printf("|%s| => ", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  printf("|%d|%s|\n", val, str);
  assert(result == RESULT_OK);
  assert(val == INT32_MIN);
  assert(0 == strcmp(str, "gh"));

  str = "  02147483648abc";
  printf("|%s| =>\n", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  assert(0 == strcmp(result, "CMDARG: value out of range"));

  str = "  0xabcdefab";
  printf("|%s| =>\n", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  assert(0 == strcmp(result, "CMDARG: value out of range"));

  str = "  02147483647abc";
  printf("|%s| =>\n", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX - 1, &val);
  assert(0 == strcmp(result, "CMDARG: value out of range"));

  str = "  -02147483648abc";
  printf("|%s| =>\n", str);
  result = parseInt(&str, INT32_MIN + 1, INT32_MAX, &val);
  assert(0 == strcmp(result, "CMDARG: value out of range"));

  str = "  ^02147483648abc";
  printf("|%s| =>\n", str);
  result = parseInt(&str, INT32_MIN, INT32_MAX, &val);
  assert(0 == strcmp(result, "CMDARG: expected integer"));
}

static void testParseOnOff(void) {
  const char *str;
  result_t result;
  bool val;

  str = "  \"ON";
  result = parseOnOff(&str, &val);
  assert(0 == strcmp(result, "CMDARG: missing ending quote"));

  str = "  ONF";
  result = parseOnOff(&str, &val);
  assert(0 == strcmp(result, "CMDARG: expected ON or OFF"));

  str = " OF";
  result = parseOnOff(&str, &val);
  assert(0 == strcmp(result, "CMDARG: expected ON or OFF"));

  str = "O";
  result = parseOnOff(&str, &val);
  assert(0 == strcmp(result, "CMDARG: expected ON or OFF"));

  str = "\"ON\"";
  result = parseOnOff(&str, &val);
  assert(result == RESULT_OK);
  assert(val);

  str = "\"OFF\"";
  result = parseOnOff(&str, &val);
  assert(result == RESULT_OK);
  assert(!val);
}

static void testParseString(void) {
#define BUF_LEN 1024
  const char *str;
  result_t result;
  static char buf[BUF_LEN];

  str = "  TEST\"StrR   ";
  printf("|%s| => ", str);
  result = parseString(&str, BUF_LEN, buf);
  printf("|%s|%s|\n", buf, str);
  assert(result == RESULT_OK);
  assert(0 == strcmp(buf, "TEST\"StrR"));
  assert(0 == strcmp(str, "  "));

  str = "  TEST\"StrR";
  printf("|%s| => ", str);
  result = parseString(&str, BUF_LEN, buf);
  printf("|%s|%s|\n", buf, str);
  assert(result == RESULT_OK);
  assert(0 == strcmp(buf, "TEST\"StrR"));
  assert(0 == strcmp(str, ""));

  str = "   \"\\\"TES\\\\T\"StrR   ";
  printf("|%s| => ", str);
  result = parseString(&str, BUF_LEN, buf);
  printf("|%s|%s|\n", buf, str);
  assert(result == RESULT_OK);
  assert(0 == strcmp(buf, "\"TES\\T"));
  assert(0 == strcmp(str, "StrR   "));

  str = "    \"TESTStrR \\\"  ";
  printf("|%s| => \n", str);
  result = parseString(&str, BUF_LEN, buf);
  assert(0 == strcmp(result, "CMDARG: missing ending quote"));

#undef BUF_LEN
}

void testCmdArg(void) {
  testParseInt();
  testParseOnOff();
  testParseString();

  printf("CmdArg tests passed\n\n");
}

#endif // UNITTEST
