#include <ctype.h>

#include "types.h"

int stricmp_P(const char *str1, immutable_str str2) {
  char c1, c2;
  for (;;) {
    c1 = tolower(*str1);
    c2 = tolower(READ_IMMUTABLE_BYTE(str2));
    if (c1 < c2)
      return -1;
    if (c1 > c2)
      return 1;
    if (!c1)
      return 0;
    ++str1;
    ++str2;
  }
}

#ifdef UNITTEST

#include <assert.h>

static void testStricmp(void) {
  assert(stricmp_P("aBcDEfgH", S("AbCDefgH")) == 0);
  assert(stricmp_P("aBcDEfgH", S("AbCDefg")) > 0);
  assert(stricmp_P("aBcDEfg", S("AbCDefgH")) < 0);
}

#endif
