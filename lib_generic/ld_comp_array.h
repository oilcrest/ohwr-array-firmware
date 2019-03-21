/**
 *  \file
 *
 *  \brief Macros for building link time composed arrays in GNU toolchain.
 *
 *  \note Requires adding <tt>*(SORT_BY_NAME(.ld_comp_array*))</tt> in read-only
 * data section in linker script. The default linker script may be displayed
 * using <tt>(optional-cross-compile-prefix-)ld --verbose</tt>.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _LD_COMP_ARRAY_H__
#define _LD_COMP_ARRAY_H__

#define LD_COMP_ARRAY(_name)                                                   \
  __attribute__((section(".ld_comp_array_" #_name "_data")))

#define LD_COMP_ARRAY_DECLARE(_name, _type, ...)                               \
  _type __attribute__((section(".ld_comp_array_" #_name "_begin")))            \
      _name[] = {};                                                            \
  _type __attribute__((section(".ld_comp_array_" #_name "_end")))              \
      _name##_end[] = {__VA_ARGS__}

#define LD_COMP_ARRAY_EXTERN(_name, _type)                                     \
  extern _type __attribute__((section(".ld_comp_array_" #_name "_begin")))     \
      _name[]

#endif // !_LD_COMP_ARRAY_H__
