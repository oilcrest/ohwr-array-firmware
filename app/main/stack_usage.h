/**
 *  \file
 *
 *  \brief Trivial peak stack usage measurement.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _STACK_USAGE_H__
#define _STACK_USAGE_H__

#include "types.h"
#include "ucos_ii.h"

/** \brief Fill the whole stack with predictable pattern.
 *
 *  \param[in]  stack Array representing stack to initialize.
 *  \param[in]  size  Number of OS_STK elements.
 *
 *  \note Must be called before the task using this stack is started.
 */
void StackUsage_Fill(OS_STK *stack, size_t size);

/** \brief Measure peak stack usage.
 *
 *  \param[in]  stack Array representing stack to measure.
 *  \return     Number of stack elements probably never written to.
 *
 *  \note Assumed stack growing in descending order of addresses.
 *
 *  \note The result may be wrong if application put values
 *        matching the pattern written in StackUsage_Fill.
 */
size_t StackUsage_Peak(OS_STK *stack);

#endif //!_STACK_USAGE_H__
