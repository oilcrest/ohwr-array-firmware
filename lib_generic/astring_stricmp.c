#include <ctype.h>

int stricmp(const char *str1, const char *str2) {
  char c1, c2;
  for (;;) {
    c1 = tolower(*str1);
    c2 = tolower(*str2);
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
