#include <avr/pgmspace.h>
#include "dma_driver.h"
#include "mt.h"
#include "dma_alloc.h"
#include "types.h"
// #include "debug.h"

static uint8_t freeChannels = 0x0F;
static void (*isrs[4])(void *);
static void *isrObjs[4];

static volatile DMA_CH_t * const channels[] PROGMEM = { &DMA.CH0, &DMA.CH1, &DMA.CH2, &DMA.CH3 };

#define DMA_ISR(_ch) \
MT_ISR(DMA_CH ## _ch ## _vect) \
{ \
	(*isrs[_ch])(isrObjs[_ch]); \
}

DMA_ISR(0)
DMA_ISR(1)
DMA_ISR(2)
DMA_ISR(3)

volatile DMA_CH_t *DMA_AllocChannel(void (*isr)(void *), void *pObj)
{
	uint8_t k = 0;
//	DPRINTF("freeChannels=%02X\n", freeChannels);
	uint8_t fc = freeChannels;
	while (fc)
	{
		if (fc & 1)
		{
			freeChannels &= ~(1 << k);
			// enable DMA subsystem on first request
			// simplified - won't work if freeChannel is implemented
			if (k == 0)
				DMA_Enable();
			isrs[k] = isr;
			isrObjs[k] = pObj;
			return (volatile DMA_CH_t *)READ_IMMUTABLE_PTR(&channels[k]);
		}
		fc >>= 1;
		k++;
	}
	return NULL;
}
