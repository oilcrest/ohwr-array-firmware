/**
 *  \file
 *
 *  \brief Endianness definitions for Atmel AVR. Based on
 *         endian.h and bits/endian.h from GNU C library.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _ENDIAN_H__
#define _ENDIAN_H__

/* Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321

/* Convention for AVR and AVR-GCC */
#define __BYTE_ORDER __LITTLE_ENDIAN

#endif // !_ENDIAN_H__
