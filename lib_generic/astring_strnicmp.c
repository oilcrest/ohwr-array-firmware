#include <ctype.h>

#include "types.h"

int strnicmp(const char *str1, const char *str2, size_t count) {
  char c1, c2;
  while (count--) {
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
  return 0;
}
