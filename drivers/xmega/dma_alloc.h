/**
 *  \file
 *
 *  \brief Simple DMA channel allocation.
 */
#ifndef _DMA_ALLOC_H__
#define _DMA_ALLOC_H__

#include <avr/io.h>

/**
 *  Return pointer to first free DMA channel structure,
 *  or NULL, when no free channels available.
 */
volatile DMA_CH_t *DMA_AllocChannel(void (*isr)(void *), void *pObj);

#endif // !_DMA_ALLOC_H__
