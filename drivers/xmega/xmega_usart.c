#include "clksys_getfreq.h"
#include "pmic_driver.h"
#include "dma_driver.h"
#include "dma_alloc.h"
#include "mt.h"
#include "debug.h"
#include "app_cfg.h"
#include "serial.h"

#define PORTX_USART0_XCK (0x01 << 1)       // USART 0 Port C/D/E/F  pins settings
#define PORTX_USART0_RX (0x01 << 2)
#define PORTX_USART0_TX (0x01 << 3)

#define PORTX_USART1_XCK (0x01 << 5)       // USART 1 Port C/D/E/F  pins settings
#define PORTX_USART1_RX (0x01 << 6)
#define PORTX_USART1_TX (0x01 << 7)

typedef struct USART_HwProp_struct
{
	volatile USART_t *pUsart;
	volatile PORT_t *pPort;
	uint8_t xck_bm;
	uint8_t rx_bm;
	uint8_t tx_bm;
	uint8_t triggerSource;
}
__attribute__((aligned))
USART_HwProp;

USART_HwProp const gHwProps[8] PROGMEM =
{

#ifdef XMEGA_USART_ENABLE_USARTC0
	{ &USARTC0, &PORTC, PORTX_USART0_XCK, PORTX_USART0_RX, PORTX_USART0_TX, DMA_CH_TRIGSRC_USARTC0_DRE_gc },
#define USARTC0_ENABLED 1
#else
#define USARTC0_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTC1
	{ &USARTC1, &PORTC, PORTX_USART1_XCK, PORTX_USART1_RX, PORTX_USART1_TX, DMA_CH_TRIGSRC_USARTC1_DRE_gc },
#define USARTC1_ENABLED 1
#else
#define USARTC1_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTD0
	{ &USARTD0, &PORTD, PORTX_USART0_XCK, PORTX_USART0_RX, PORTX_USART0_TX, DMA_CH_TRIGSRC_USARTD0_DRE_gc },
#define USARTD0_ENABLED 1
#else
#define USARTD0_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTD1
	{ &USARTD1, &PORTD, PORTX_USART1_XCK, PORTX_USART1_RX, PORTX_USART1_TX, DMA_CH_TRIGSRC_USARTD1_DRE_gc },
#define USARTD1_ENABLED 1
#else
#define USARTD1_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTE0
	{ &USARTE0, &PORTE, PORTX_USART0_XCK, PORTX_USART0_RX, PORTX_USART0_TX, DMA_CH_TRIGSRC_USARTE0_DRE_gc },
#define USARTE0_ENABLED 1
#else
#define USARTE0_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTE1
	{ &USARTE1, &PORTE, PORTX_USART1_XCK, PORTX_USART1_RX, PORTX_USART1_TX, DMA_CH_TRIGSRC_USARTE1_DRE_gc },
#define USARTE1_ENABLED 1
#else
#define USARTE1_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTF0
	{ &USARTF0, &PORTF, PORTX_USART0_XCK, PORTX_USART0_RX, PORTX_USART0_TX, DMA_CH_TRIGSRC_USARTF0_DRE_gc },
#define USARTF0_ENABLED 1
#else
#define USARTF0_ENABLED 0
#endif

#ifdef XMEGA_USART_ENABLE_USARTF1
	{ &USARTF1, &PORTF, PORTX_USART1_XCK, PORTX_USART1_RX, PORTX_USART1_TX, DMA_CH_TRIGSRC_USARTF1_DRE_gc }
#define USARTF1_ENABLED 1
#else
#define USARTF1_ENABLED 0
#endif

};

#define USARTC0_NUM 0
#define USARTC1_NUM (USARTC0_NUM + USARTC0_ENABLED)
#define USARTD0_NUM (USARTC1_NUM + USARTC1_ENABLED)
#define USARTD1_NUM (USARTD0_NUM + USARTD0_ENABLED)
#define USARTE0_NUM (USARTD1_NUM + USARTD1_ENABLED)
#define USARTE1_NUM (USARTE0_NUM + USARTE0_ENABLED)
#define USARTF0_NUM (USARTE1_NUM + USARTE1_ENABLED)
#define USARTF1_NUM (USARTF0_NUM + USARTF0_ENABLED)

#define USART_COUNT (USARTF1_NUM + USARTF1_ENABLED)

/** \brief Return pointer to USART_t structure from given row of gHwProps array.
 *
 *  \param[in] _pProp Pointer to row. May be obtained as &gHwProps[usart],
 *                    where usart is USART identifier.
 */
#define PROP_USART(_pProp) ((volatile USART_t *)pgm_read_word_near(&((_pProp)->pUsart)))

/** \brief Return pointer to PORT_t structure from given row of gHwProps array.
 *
 *  \param[in] _pProp Pointer to row. May be obtained as &gHwProps[usart],
 *                    where usart is USART identifier.
 */
#define PROP_PORT(_pProp)  ((volatile PORT_t *)pgm_read_word_near(&((_pProp)->pPort)))

/** \brief Enable one of USART interrupts for given USART.
 *
 *  \param[in] _usart USART identifier.
 *  \param[in] _int   Interrupt name, one of: RXC, TXC, DRE.
 */
#define USART_ENABLE_INTERRUPT(_usart, _int) \
	do { \
		PROP_USART(&gHwProps[_usart])->CTRLA |= USART_ ## _int ## INTLVL_HI_gc; \
	} while (0)

/** \brief Disable one of USART interrupts for given USART.
 *
 *  \param[in] _usart USART identifier.
 *  \param[in] _int   Interrupt name, one of: RXC, TXC, DRE.
 */
#define USART_DISABLE_INTERRUPT(_usart, _int) \
	do { \
		PROP_USART(&gHwProps[_usart])->CTRLA &= (uint8_t)(~USART_ ## _int ## INTLVL_gm); \
	} while (0)

/** \brief Setup port and USART for serial transmision.
 *
 *  Setup direction of TX/RX pins of port associated with
 *  specified USART: Set Px7/3 (TXDn) state as high, set Px7/3
 *  (TXDn) as output pin, and set Px6/3 (RXDn) as input pin.
 *  Disable USART interrupts. Set mode to 8N1. Set baud rate.
 *  Enable transmitter and receiver.
 *
 *  \param[in] usart    USART identifier.
 *
 *  \param[in] baudRate Baud rate to be set.
 *
 *  \return Pointer to USART_t structure of USART corresponding to
 *          specified identifier, or NULL when invalid port
 *          identifier was specified.
 */
static volatile USART_t *setupUsart(uint8_t usart, uint32_t baudRate)
{
	volatile USART_t *pUsart;
	volatile PORT_t *pPort;
	USART_HwProp const *pProp;

	if (usart >= USART_COUNT)
		return NULL;

	pProp = &gHwProps[usart];
	pUsart = PROP_USART(pProp);
	pPort = PROP_PORT(pProp);

	// disable interrupts, receiver and transceiver

	pUsart->CTRLA = USART_RXCINTLVL_OFF_gc
		| USART_DREINTLVL_OFF_gc
		| USART_TXCINTLVL_OFF_gc;

	pUsart->CTRLB = 0;

	// configure with new parameters
	uint8_t tx_bm = pgm_read_byte_near(&pProp->tx_bm);
	pPort->OUTSET = tx_bm;
	pPort->DIRSET = tx_bm;
	pPort->DIRCLR = pgm_read_byte_near(&pProp->rx_bm);

	uint32_t perFreq = CLKSYS_GetFrequency(CLKSYS_OUTPUT_PER);
	uint32_t clkDiv = perFreq / baudRate;
	if (clkDiv < 16 && (perFreq * clkDiv != baudRate))
	{
		uint8_t xck_bm = pgm_read_byte_near(&pProp->xck_bm);
		pPort->OUTSET = xck_bm;
		pPort->DIRSET = xck_bm;
		clkDiv = ((perFreq / 2 / baudRate) - 1);
		pUsart->CTRLC = USART_CMODE_SYNCHRONOUS_gc
			| USART_PMODE_DISABLED_gc
			| USART_CHSIZE_8BIT_gc;
	}
	else
	{
		if (clkDiv < 256)
			clkDiv = (clkDiv - 16) | 0xC000; // using bscale = -4
		else
			clkDiv = ((perFreq / baudRate) - 1) / 16;
		pUsart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc
			| USART_PMODE_DISABLED_gc
			| USART_CHSIZE_8BIT_gc;
	}

	pUsart->BAUDCTRLA = (uint8_t)(clkDiv & 0xFF);
	pUsart->BAUDCTRLB = (uint8_t)(clkDiv >> 8);

	pUsart->CTRLB = USART_RXEN_bm | USART_TXEN_bm;

	return pUsart;
}

typedef struct SerialInfo_struct
{
	volatile USART_t *pUsart;
	ByteFifo *pInFifo;
	ByteFifo *pOutFifo;
	MT_SemType inFifoSem;
	MT_SemType outFifoSem;
	volatile DMA_CH_t *pDMA;
}
SerialInfo;

static SerialInfo serialInfo[8];

#define SERIAL_RXC_ISR(_num, _name) \
MT_ISR(USART ## _name ## _RXC_vect) \
{ \
	if (ByteFifo_Put(serialInfo[_num].pInFifo, USART ## _name.DATA)) \
		MT_SEM_POST(serialInfo[_num].inFifoSem); \
}

#ifdef XMEGA_USART_ENABLE_USARTC0
SERIAL_RXC_ISR(USARTC0_NUM, C0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTC1
SERIAL_RXC_ISR(USARTC1_NUM, C1)
#endif
#ifdef XMEGA_USART_ENABLE_USARTD0
SERIAL_RXC_ISR(USARTD0_NUM, D0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTD1
SERIAL_RXC_ISR(USARTD1_NUM, D1)
#endif
#ifdef XMEGA_USART_ENABLE_USARTE0
SERIAL_RXC_ISR(USARTE0_NUM, E0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTE1
SERIAL_RXC_ISR(USARTE1_NUM, E1)
#endif
#ifdef XMEGA_USART_ENABLE_USARTF0
SERIAL_RXC_ISR(USARTF0_NUM, F0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTF1
SERIAL_RXC_ISR(USARTF1_NUM, F1)
#endif

#define SERIAL_TXC_ISR(_num, _name) \
MT_ISR(USART ## _name ## _TXC_vect) \
{ \
	USART_DISABLE_INTERRUPT(_num, TXC); \
	MT_SEM_POST(serialInfo[_num].outFifoSem); \
}

#ifdef XMEGA_USART_ENABLE_USARTC0
SERIAL_TXC_ISR(USARTC0_NUM, C0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTC1
SERIAL_TXC_ISR(USARTC1_NUM, C1)
#endif
#ifdef XMEGA_USART_ENABLE_USARTD0
SERIAL_TXC_ISR(USARTD0_NUM, D0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTD1
SERIAL_TXC_ISR(USARTD1_NUM, D1)
#endif
#ifdef XMEGA_USART_ENABLE_USARTE0
SERIAL_TXC_ISR(USARTE0_NUM, E0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTE1
SERIAL_TXC_ISR(USARTE1_NUM, E1)
#endif
#ifdef XMEGA_USART_ENABLE_USARTF0
SERIAL_TXC_ISR(USARTF0_NUM, F0)
#endif
#ifdef XMEGA_USART_ENABLE_USARTF1
SERIAL_TXC_ISR(USARTF1_NUM, F1)
#endif

static void Serial_DMA_ISR(void *pObj)
{
	volatile DMA_CH_t *pDMA = ((SerialInfo *)pObj)->pDMA;
	if (pDMA->CTRLB & DMA_CH_ERRIF_bm)
	{
		DPRINTF("DMA ERROR\n");
		pDMA->CTRLB |= DMA_CH_ERRIF_bm;
	}
	else
	{
		pDMA->CTRLB |= DMA_CH_TRNIF_bm;
	}
	// enable TXC interrupt
	((SerialInfo *)pObj)->pUsart->CTRLA |= USART_TXCINTLVL_HI_gc;
}

result_t Serial_Init(uint8_t usart, uint32_t baudrate, ByteFifo *pInFifo, ByteFifo *pOutFifo, int options)
{
	if (options & ~SERIAL_USE_TX_DMA)
	{
		return S("Serial_Init: Unsupported options");
	}
	if (options & SERIAL_USE_TX_DMA)
	{
		if (!(serialInfo[usart].pDMA = DMA_AllocChannel(&Serial_DMA_ISR, (void *)&serialInfo[usart])))
			return S("Serial_Init: No DMA channels available");
		DMA_SetIntLevel(serialInfo[usart].pDMA, DMA_CH_TRNINTLVL_HI_gc, DMA_CH_ERRINTLVL_HI_gc);
	}
	else
	{
		serialInfo[usart].pDMA = NULL;
	}
//	DPRINTF("SERIAL OPEN %d,%ld,%p,%p,%p\n", usart, baudrate, pInFifo, pOutFifo, serialInfo[usart].pDMA);
	volatile USART_t *pUsart = setupUsart(usart, baudrate);
	serialInfo[usart].pUsart = pUsart;
	MT_SEM_INIT(serialInfo[usart].inFifoSem, 0);
	MT_SEM_INIT(serialInfo[usart].outFifoSem, 1);
	serialInfo[usart].pInFifo = pInFifo;
	serialInfo[usart].pOutFifo = pOutFifo;
	if (pOutFifo && SERIAL_USE_TX_DMA)
	{
		return S("Serial_Init: pOutFifo is needless when using DMA");
	}
	if (pInFifo)
	{
		pUsart->CTRLA |= USART_RXCINTLVL_HI_gc;
	}
	else
	{
//		DPRINTF("Warning: RX disabled\n");
	}
	PMIC_EnableHighLevel();
	return RESULT_OK;
}

void Serial_Putc(uint8_t usart, char k)
{
	Serial_Write(usart, &k, 1);
}

void Serial_Flush(uint8_t usart)
{
	MT_SEM_PEND(serialInfo[usart].outFifoSem, 0);
	MT_SEM_POST(serialInfo[usart].outFifoSem);
}

void Serial_Write(uint8_t usart, const void *buf, size_t length)
{
	volatile DMA_CH_t *pDMA = serialInfo[usart].pDMA;
	if (length == 0)
		return;
	MT_SEM_PEND(serialInfo[usart].outFifoSem, 0);
	if (length == 1)
	{
		serialInfo[usart].pUsart->DATA = *(uint8_t *)buf;
		serialInfo[usart].pUsart->CTRLA |= USART_TXCINTLVL_HI_gc;
		return;
	}
	DMA_SetupBlock(
		pDMA,
		buf,
		DMA_CH_SRCRELOAD_NONE_gc,
		DMA_CH_SRCDIR_INC_gc,
		(void *)&PROP_USART(&gHwProps[usart])->DATA,
		DMA_CH_DESTRELOAD_NONE_gc,
		DMA_CH_DESTDIR_FIXED_gc,
		length,
		DMA_CH_BURSTLEN_1BYTE_gc,
		0, // Perform once
		false
	);
	DMA_EnableSingleShot(pDMA);
	// USART Trigger source, Data Register Empty
	DMA_SetTriggerSource(pDMA, pgm_read_byte_near(&gHwProps[usart].triggerSource));
	DMA_EnableChannel(pDMA);
}

char Serial_Getc(uint8_t usart)
{
	MT_SEM_PEND(serialInfo[usart].inFifoSem, 0);
	USART_DISABLE_INTERRUPT(usart, RXC);
	char q = ByteFifo_Get(serialInfo[usart].pInFifo);
	USART_ENABLE_INTERRUPT(usart, RXC);
	return q;
}

/*
 * Debug console implementation
 */

#ifndef DISABLE_DEBUG

#ifndef DEBUG_USART
#error "DEBUG_USART not defined"
#endif

#ifndef DEBUG_USART_BAUDRATE
#error "DEBUG_USART_BAUDRATE not defined"
#endif

#include <stdarg.h>
#include "printf.h"

volatile USART_t* Debug_pUsart;

static void Debug_PutChar(void *pArg, char c)
{
	if (c == '\n')
		Debug_PutChar(pArg, '\r');
	while ((Debug_pUsart->STATUS & USART_DREIF_bm) == 0) {}
    Debug_pUsart->DATA = c;
}

void Debug_Flush(void)
{
	// this doesn't work as expected, why?
	// while ((Debug_pUsart->STATUS & USART_TXCIF_bm) == 0) {}

	// TODO: replace this delay loop with _working_ wait for
	// all bits to leave the port.
	// bad, dirty, ugly workaround
	uint32_t i;
	for (i = 0; i < 0x1000U; ++i) { nop(); }
}

void Debug_Init(void)
{
	Debug_pUsart = setupUsart(DEBUG_USART, DEBUG_USART_BAUDRATE);
}

void Debug_Putc(char c)
{
	while ((Debug_pUsart->STATUS & USART_DREIF_bm) == 0) {}
    Debug_pUsart->DATA = c;
}

void Debug_Printf_P(const prog_char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	kvprintf_P(fmt, &Debug_PutChar, 0, ap);
	va_end(ap);
	Debug_Flush();
}

#endif // !DISABLE_DEBUG

