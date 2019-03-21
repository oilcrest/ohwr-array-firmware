/**
 *  \file
 *
 *  \brief Driver for getting current clock frequencies.
 *
 *  This driver is an extension to ATMEL's XMEGA Clock System driver,
 *  providing functions for getting current clock frequencies in Hz.
 *  It is useful when some clock frequency dependent quantities
 *  must be set for peripherals, such as timers or USARTs.
 *
 *  \note If the target system uses (or has an option to use) external
 *        oscillator, its frequency should be defined as
 *        CLKSYS_XOSC_FREQUENCY in app_cfg.h file.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */
 
#ifndef _CLKSYS_GETFREQ_H__
#define _CLKSYS_GETFREQ_H__

#include <stdint.h>
#include "clksys_driver.h"

/** \brief System clock output enumeration.
 */
typedef enum CLKSYS_Output_enum
{
	CLKSYS_OUTPUT_PER,     ///< Peripheral clock
	CLKSYS_OUTPUT_PER2,    ///< Peripheral clock 2 (before prescaler C)
	CLKSYS_OUTPUT_PER4,    ///< Peripheral clock 4 (before prescaler B)
	CLKSYS_OUTPUT_CPU      ///< CPU clock
} CLKSYS_Output;

/** \brief This function gets the current main system clock source.
 *
 *  \return  Clock source currently used as input for the system clock
 *           prescaler block.
 */
static inline CLK_SCLKSEL_t CLKSYS_Main_ClockSource_Get(void)
{
	return (CLK_SCLKSEL_t)(CLK.CTRL & CLK_SCLKSEL_gm);
}

/** \brief Get frequency of the main system clock.
 *
 *  \return    System clock frequency in Hz.
 */
uint32_t CLKSYS_Main_ClockSource_GetFrequency(void);

/** \brief Get frequency of the clock signal specified by clkOut parameter.
 *
 *  \param[in] clkOut Clock output to get its frequency.
 *
 *  \return    Frequency of specified clock signal in Hz.
 */
uint32_t CLKSYS_GetFrequency(CLKSYS_Output clkOut);

/** \brief Get frequency of CPU clock.
 *
 *  \return    Current CPU frequency in Hz.
 */
static inline uint32_t CLKSYS_GetCpuFrequency(void)
{
	return CLKSYS_GetFrequency(CLKSYS_OUTPUT_CPU);
}

#endif // !_CLKSYS_GETFREQ_H__

