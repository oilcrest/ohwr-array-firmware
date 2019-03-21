#include "app_cfg.h"
#include "clksys_getfreq.h"

#ifndef CLKSYS_XOSC_FREQUENCY
#warning "CLKSYS_XOSC_FREQUENCY not configured. Setting to default value of 0."
#define CLKSYS_XOSC_FREQUENCY 0
#endif

uint32_t CLKSYS_Main_ClockSource_GetFrequency(void)
{
	uint8_t sysClkSrc = CLKSYS_Main_ClockSource_Get();

	switch (sysClkSrc)
	{
	case CLK_SCLKSEL_RC2M_gc:
		return 2000000;
	case CLK_SCLKSEL_RC32M_gc:
		return 32000000;
	case CLK_SCLKSEL_RC32K_gc:
		return 32768;
	case CLK_SCLKSEL_XOSC_gc:
		return CLKSYS_XOSC_FREQUENCY;
	case CLK_SCLKSEL_PLL_gc:
		return 0; // SYSCLK_PLL_GetFrequency()
	default:
		return 0;
	}
}

uint32_t CLKSYS_GetFrequency(CLKSYS_Output clkOut)
{
	uint32_t sysClkFreq = CLKSYS_Main_ClockSource_GetFrequency();
	uint8_t regVal = CLK.PSCTRL;
	uint8_t diva;
	uint8_t divb;
	uint8_t divc;

	regVal = CLK.PSCTRL;

	switch ((regVal & CLK_PSADIV_gm) >> CLK_PSADIV_gp)
	{
	case CLK_PSADIV_1_gc: diva = 0; break;
	case CLK_PSADIV_2_gc: diva = 1; break;
	case CLK_PSADIV_4_gc: diva = 2; break;
	case CLK_PSADIV_8_gc: diva = 3; break;
	case CLK_PSADIV_16_gc: diva = 4; break;
	case CLK_PSADIV_32_gc: diva = 5; break;
	case CLK_PSADIV_64_gc: diva = 6; break;
	case CLK_PSADIV_128_gc: diva = 7; break;
	case CLK_PSADIV_256_gc: diva = 8; break;
	case CLK_PSADIV_512_gc: diva = 9; break;
	default:
		return 0;
	}

	switch ((regVal & CLK_PSBCDIV_gm) >> CLK_PSBCDIV_gp)
	{
	case CLK_PSBCDIV_1_1_gc: divb = 0; divc = 0; break;
	case CLK_PSBCDIV_1_2_gc: divb = 0; divc = 1; break;
	case CLK_PSBCDIV_4_1_gc: divb = 2; divc = 0; break;
	case CLK_PSBCDIV_2_2_gc: divb = 1; divc = 1; break;
	default:
		return 0;
	}

	switch (clkOut)
	{
	case CLKSYS_OUTPUT_PER4:
		return sysClkFreq >> diva;
	case CLKSYS_OUTPUT_PER2:
		return sysClkFreq >> (diva + divb);
	case CLKSYS_OUTPUT_PER:
	case CLKSYS_OUTPUT_CPU:
		return sysClkFreq >> (diva + divb + divc);
	default:
		return 0;
	}
}

