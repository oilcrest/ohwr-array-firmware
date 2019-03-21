#include "adc_driver.h"

#include "mt.h"
#include "sp_driver.h"
#include <stddef.h>

void ADC_Init(void)
{
	ADCA.CALL = SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	ADCA.CALH = SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
	ADCB.CALL = SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCBCAL0));
	ADCB.CALH = SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCBCAL1));

	ADCA.CTRLA = ADC_ENABLE_bm;
	ADCB.CTRLA = ADC_ENABLE_bm;

	// measure in signed mode
	ADCA.CTRLB = ADC_CONMODE_bm;
	ADCB.CTRLB = ADC_CONMODE_bm;

	ADCA.REFCTRL = ADC_TEMPREF_bm | ADC_BANDGAP_bm;
	ADCB.REFCTRL = ADC_TEMPREF_bm | ADC_BANDGAP_bm;

	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
}

int16_t ADC_Read(volatile ADC_t *pADC, ADC_CH_INPUTMODE_t mode, uint8_t muxctrl)
{
	pADC->CH0.CTRL = (uint8_t)mode;
	pADC->CH0.MUXCTRL = muxctrl;
	pADC->CTRLA |= ADC_CH0START_bm;
	pADC->CTRLA |= ADC_ENABLE_bm;
	MT_SLEEPMS(100);
	while ((pADC->CH0.INTFLAGS & ADC_CH_CHIF_bm) == 0)
		MT_SLEEPMS(10);
	uint8_t resl = pADC->CH0.RESL;
	uint8_t resh = pADC->CH0.RESH;
	int16_t result = ((int16_t)resh << 8) | resl;
	return result;
}

