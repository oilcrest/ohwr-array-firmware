/**
 *  \file
 *
 *  \brief ADC driver for AVR XMEGA.
 *
 *  \author Adrian Matoga
 */
#ifndef _ADC_DRIVER_H__
#define _ADC_DRIVER_H__

#include <avr/io.h>

/** \brief Initialize both XMEGA ADCs.
 *
 *  This function enables ADCA and ADCB blocks and copies
 *  calibration values from production signature row to
 *  ADC calibration registers.
 */
void ADC_Init(void);

/** \brief Blocking read from AVR XMEGA ADC.
 *
 *  \param[in] pADC    ADC to use.
 *  \param[in] mode    Input mode. Either ADC_CH_INPUTMODE_SINGLEENDED_gc or ADC_CH_INPUTMODE_INTERNAL_gc.
 *  \param[in] muxctrl Mux selection, mode dependent - ADC_CH_MUXINT_* or ADC_CH_MUXPOS_*.
 *
 *  \return    Raw 12-bit value from ADC, right-aligned.
 */
int16_t ADC_Read(volatile ADC_t *pADC, ADC_CH_INPUTMODE_t mode, uint8_t muxctrl);

#endif // !_ADC_DRIVER_H__

