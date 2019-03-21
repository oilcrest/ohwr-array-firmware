/**
 *  \file
 *
 *  \brief Stack usage monitor
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#include "stack_usage.h"

void StackUsage_Fill(OS_STK *stack, size_t size) {
  for (size_t i = 0; i < size; ++i)
    stack[i] = (OS_STK)i ^ (OS_STK)0x55555555;
}

size_t StackUsage_Peak(OS_STK *stack) {
  size_t i = 0;
  while (stack[i] == ((OS_STK)i ^ (OS_STK)0x55555555)) {
    ++i;
  }
  return i;
}
