#include "types.h"
#include <ctype.h>

int strnicmp_P(const char *str1, immutable_str str2, size_t count) {
  char c1, c2;
  while (count--) {
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
  return 0;
}

#ifdef UNITTEST

static void testStricmp(void) {
  /*	assert(stricmp("aBcDEfgH", "AbCDefgH") == 0);
          assert(stricmp("aBcDEfgH", "AbCDefg") > 0);
          assert(stricmp("aBcDEfg", "AbCDefgH") < 0);*/

  /*	assert(strnicmp("aBcDEfgHIJ", "AbCDefgH", 8) == 0);
          assert(strnicmp("aBcDEfgHIJ", "AbCDefgH", 9) > 0);
          assert(strnicmp("aBcDEfgH", "AbCDefghI", 9) < 0);
          assert(strnicmp("aBcDEfgH", "AbCDefghI", 8) == 0);
          assert(strnicmp("aBcDEfgH", "AbCDefgh", 9) == 0);*/

  assert(strnicmp_P("aBcDEfgHIJ", S("AbCDefgH"), 8) == 0);
  assert(strnicmp_P("aBcDEfgHIJ", S("AbCDefgH"), 9) > 0);
  assert(strnicmp_P("aBcDEfgH", S("AbCDefghI"), 9) < 0);
  assert(strnicmp_P("aBcDEfgH", S("AbCDefghI"), 8) == 0);
  assert(strnicmp_P("aBcDEfgH", S("AbCDefgh"), 9) == 0);
}

#endif // UNITTEST
