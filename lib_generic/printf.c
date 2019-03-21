/** \file
 *
 *  \brief Minimal universal *printf - implementation.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#include "printf.h"
#include "astring.h"

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned short u_short;

// longest number supported by printf
typedef long myintmax_t;
typedef u_long myuintmax_t;

const char IMMUTABLE_MEM hex2ascii_data[] =
    "0123456789abcdefghijklmnopqrstuvwxyz";
#define hex2ascii(hex) (READ_IMMUTABLE_BYTE(hex2ascii_data + (hex)))

#define toupper(c) ((c)-0x20 * (((c) >= 'a') && ((c) <= 'z')))

/** Max number conversion buffer length: a myintmax_t in base 2, plus NUL byte.
 */
#define MAXNBUF (sizeof(myintmax_t) * 8 + 1)

/** \brief Put a 0-terminated ASCII number (base <= 36)
 *         in a buffer in reverse order.
 *
 *  \param[in]  num     Number to be printed.
 *  \param[in]  base    Numeric base.
 *  \aramm[in]  upper   true if the number is to be printed in uppercase.
 *  \param[out] pLen	(optional) Number of chars printed, including
 * terminating '\0'. \param[out] nbuf    Output buffer.
 *
 *  \return Pointer to the last character written in the buffer
 *          (i.e., the first character of the string).
 *
 *  \note   The buffer pointed to by `nbuf' must have length >= MAXNBUF.
 */
static char *ksprintn(myuintmax_t num, uint8_t base, bool upper, uint8_t *pLen,
                      char nbuf[]) {
  char *p, c;
  uint8_t k = MAXNBUF;

  p = nbuf - 1;
  do {
    c = hex2ascii(num % base);
    *(++p) = upper ? toupper(c) : c;
  } while ((num /= base) && (--k > 0));
  if (pLen)
    *pLen = p - nbuf + 1;
  return p;
}

typedef union {
  uint32_t L;
  float F;
} LF_t;

/** \author cb30 http://www.edaboard.com/thread5585.html#post21957
 */
static char *ftoa(float f, char *outbuf) {
  uint32_t mantissa, int_part, frac_part;
  short exp2;
  LF_t x;
  char *p = outbuf;

  x.F = f;
  if (!(x.L & 0x7FFFFFFF)) {
    *p = '0';
    return p;
  }

  exp2 = (uint8_t)(x.L >> 23) - 127;
  mantissa = (x.L & 0xFFFFFF) | 0x800000;
  frac_part = 0;
  int_part = 0;

  if (exp2 >= 31 || exp2 < -23)
    return 0;
  else if (exp2 >= 23)
    int_part = mantissa << (exp2 - 23);
  else if (exp2 >= 0) {
    int_part = mantissa >> (23 - exp2);
    frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
  } else /* if (exp2 < 0) */
    frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);

  if (frac_part != 0) {
    frac_part = (uint32_t)(
        (((uint64_t)(((frac_part) << 8) | ((frac_part & 1) ? 0xFF : 0))) *
         ((uint64_t)10000000)) >>
        32);
    bool z = false;
    for (uint8_t i = 7; i > 0; --i) {
      char d = frac_part % 10;
      if (d != 0 || z) {
        z = true;
        *p++ = frac_part % 10 + '0';
      }
      frac_part /= 10;
    }
    *p++ = '.';
  }

  p = ksprintn(int_part, 10, false, NULL, p) + 1;
  if (x.L & 0x80000000)
    *p++ = '-';

  return p - 1;
}

int kvprintf_P(immutable_str fmt, void (*putChar)(void *pObj, char c),
               void *pObj, va_list ap) {
#define LFLAG 0x80
#define LADJUST 0x40
#define SHARP 0x20
#define DOT 0x10
#define SIGN 0x08
#define NEG 0x04
#define CFLAG 0x02
#define HFLAG 0x01

#define PUTCHAR(c)                                                             \
  do {                                                                         \
    char cc = (c);                                                             \
    (*putChar)(pObj, cc);                                                      \
    retval++;                                                                  \
  } while (0)

  immutable_str percent;
  char nbuf[MAXNBUF];
  const char *p;
  int ch, n;
  myuintmax_t num;
  uint8_t base;
  uint8_t flags;

  uint8_t tmp;
  int width, dwidth;
  bool upper;
  char padc;
  bool stop = false;
  int retval = 0;

  if (!fmt || !putChar)
    return 0;

  num = 0;
  for (;;) {
    padc = ' ';
    width = 0;
    while ((ch = (u_char)READ_IMMUTABLE_BYTE(fmt++)) != '%' || stop) {
      if (ch == '\0')
        return (retval);
      PUTCHAR(ch);
    }
    dwidth = 0;
    width = 0;
    percent = fmt - 1;
    upper = false;
    flags = 0;

  reswitch:
    switch (ch = (u_char)READ_IMMUTABLE_BYTE(fmt++)) {
    case '.':
      flags |= DOT;
      goto reswitch;
    case '#':
      flags |= SHARP;
      goto reswitch;
    case '+':
      flags |= SIGN;
      goto reswitch;
    case '-':
      flags |= LADJUST;
      goto reswitch;
    case '%':
      PUTCHAR(ch);
      break;
    case '*':
      if (!(flags & DOT)) {
        width = va_arg(ap, int);
        if (width < 0) {
          flags ^= LADJUST;
          width = -width;
        }
      } else {
        dwidth = va_arg(ap, int);
      }
      goto reswitch;
    case '0':
      if (!(flags & DOT)) {
        padc = '0';
        goto reswitch;
      }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      for (n = 0;; ++fmt) {
        n = n * 10 + ch - '0';
        ch = READ_IMMUTABLE_BYTE(fmt);
        if (ch < '0' || ch > '9')
          break;
      }
      if (flags & DOT)
        dwidth = n;
      else
        width = n;
      goto reswitch;
    case 'c':
      PUTCHAR(va_arg(ap, int));
      break;
    case 'd':
    case 'i':
      base = 10;
      goto handle_sign;
    case 'f': {
      float g = va_arg(ap, double);
      nbuf[0] = 0;
      p = ftoa(g, nbuf + 1);
      if (p) {
        while (*p)
          PUTCHAR(*p--);
      }
    } break;
    case 'h':
      if (flags & HFLAG) {
        flags &= (uint8_t)~HFLAG;
        flags |= CFLAG;
      } else
        flags |= HFLAG;
      goto reswitch;
    case 'l':
      flags |= LFLAG;
      goto reswitch;
    case 'n':
      if (flags & LFLAG)
        *(va_arg(ap, long *)) = retval;
      else if (flags & HFLAG)
        *(va_arg(ap, short *)) = retval;
      else if (flags & CFLAG)
        *(va_arg(ap, char *)) = retval;
      else
        *(va_arg(ap, int *)) = retval;
      break;
    case 'o':
      base = 8;
      goto handle_nosign;
    case 'p':
      base = 16;
      if (width == 0)
        flags |= SHARP;
      else
        flags &= (uint8_t)~SHARP;
      flags &= (uint8_t)~SIGN;
      num = (uintptr_t)va_arg(ap, void *);
      goto number;
    case 'S':
      upper = true;
    case 's':
      p = va_arg(ap, char *);
      if (!p) {
        p = S("(null)");
        upper = true;
      }
      if (!upper) {
        if (!(flags & DOT))
          n = strlen(p);
        else
          for (n = 0; n < dwidth && p[n]; n++) {
          }
      } else {
        if (!(flags & DOT))
          n = strlen_P(p);
        else
          for (n = 0; n < dwidth && READ_IMMUTABLE_BYTE(p + n); n++) {
          }
      }
      width -= n;
      if (!(flags & LADJUST) && width > 0)
        while (width--)
          PUTCHAR(padc);
      if (!upper) {
        while (n--)
          PUTCHAR(*p++);
      } else {
        while (n--)
          PUTCHAR(READ_IMMUTABLE_BYTE(p++));
      }
      if ((flags & LADJUST) && width > 0)
        while (width--)
          PUTCHAR(padc);
      break;
    case 'u':
      base = 10;
      goto handle_nosign;
    case 'X':
      upper = true;
    case 'x':
      base = 16;
      goto handle_nosign;
    case 'b':
      base = 2;
      goto handle_nosign;
    handle_nosign:
      flags &= (uint8_t)~SIGN;
      if (flags & LFLAG)
        num = va_arg(ap, u_long);
      else if (flags & HFLAG)
        num = (u_short)va_arg(ap, int);
      else if (flags & CFLAG)
        num = (u_char)va_arg(ap, int);
      else
        num = va_arg(ap, u_int);
      goto number;
    handle_sign:
      if (flags & LFLAG)
        num = va_arg(ap, long);
      else if (flags & HFLAG)
        num = (short)va_arg(ap, int);
      else if (flags & CFLAG)
        num = (char)va_arg(ap, int);
      else
        num = va_arg(ap, int);
    number:
      if (/*(flags & SIGN) &&*/ (myintmax_t)num < 0) {
        flags |= NEG;
        num = -(myintmax_t)num;
      }
      nbuf[0] = '\0';
      p = ksprintn(num, base, upper, &tmp, nbuf + 1);
      if ((flags & SHARP) && num != 0) {
        if (base == 8)
          tmp++;
        else if (base == 16)
          tmp += 2;
      }
      if ((flags & NEG) || (flags & SIGN))
        tmp++;

      if (!(flags & NEG) && padc != '0' && width && (width -= tmp) > 0)
        while (width--)
          PUTCHAR(padc);
      if (flags & NEG)
        PUTCHAR('-');
      else if (flags & SIGN)
        PUTCHAR('+');
      if ((flags & SHARP) && num != 0) {
        if (base == 8) {
          PUTCHAR('0');
        } else if (base == 16) {
          PUTCHAR('0');
          PUTCHAR('x');
        }
      }
      if (!(flags & LADJUST) && width && (width -= tmp) > 0)
        while (width--)
          PUTCHAR(padc);

      while (*p)
        PUTCHAR(*p--);

      if ((flags & LADJUST) && width && (width -= tmp) > 0)
        while (width--)
          PUTCHAR(padc);

      break;
    default:
      while (percent < fmt)
        PUTCHAR(READ_IMMUTABLE_BYTE(percent++));
      /*
       * Since we ignore an formatting argument it is no
       * longer safe to obey the remaining formatting
       * arguments as the arguments will no longer match
       * the format specs.
       */
      stop = true;
      break;
    }
  }

#undef LFLAG
#undef LADJUST
#undef SHARP
#undef DOT
#undef SIGN
#undef NEG
#undef CFLAG
#undef HFLAG

#undef PCHAR
}

typedef struct SnprintfBuffer_struct {
  char *buffer;
  int index;
  int bufSize;
} SnprintfBuffer;

static void snprintf_putChar(void *pObj, char c) {
#define BUFFER (((SnprintfBuffer *)pObj)->buffer)
#define INDEX (((SnprintfBuffer *)pObj)->index)
#define BUFSIZE (((SnprintfBuffer *)pObj)->bufSize)

  if (INDEX < BUFSIZE)
    BUFFER[INDEX] = c;
  ++INDEX;

#undef BUFFER
#undef INDEX
#undef BUFSIZE
}

int snprintf_P(char *buffer, int bufSize, immutable_str fmt, ...) {
  SnprintfBuffer sb = {
      .buffer = buffer,
      .index = 0,
      .bufSize = bufSize,
  };

  va_list ap;
  va_start(ap, fmt);
  int res = kvprintf_P(fmt, &snprintf_putChar, &sb, ap);
  va_end(ap);
  snprintf_putChar(&sb, '\0');
  return res;
}

#ifdef UNITTEST

#include <assert.h>
#include <stdio.h>
#include <string.h>

static char buf[1024];

#warning fix testprintf

void testPrintf(void) {
  int cnt;
  int res;
  char c = 'A';
  short h = 0x1234;
  short u = -1;
  float b = -12.125;

  const char *str = "test";
  const char *fmt =
      "%f%%%.5s%c%5d%+07i%+*hb%n%4hhx%#07X%hu%hi%10s%-10S%#o%p%q%b";
  res = snprintf_P(buf, 1024, fmt, b, "truncate", c, 123, 98765, 14, h, &cnt, c,
                   0xabc, u, u, str, str, 0177, (void *)0x12EF);

  printf("format str: %s\nresult:     %s\nlen:        %d\n", fmt, buf, res);

  assert(cnt == 43);
  assert(res == 97);
  assert(0 ==
         strcmp(buf, "-12.125%truncA   123+0098765  1001000110100   "
                     "410x000ABC65535-1      testtest      01770x12ef%q%b"));

  cnt = 0;
  res = snprintf_P(buf + 1, 25, fmt, b, "truncate", c, 123, 98765, 14, h, &cnt,
                   c, 0xabc, u, u, str, str, 0177, (void *)0x12EF);

  assert(cnt == 43);
  assert(res == 97);
  assert(0 ==
         strcmp(buf, "--12.125%truncA   123+009865  1001000110100   "
                     "410x000ABC65535-1      testtest      01770x12ef%q%b"));

  printf("printf test passed\n\n");
}

#endif // UNITTEST
