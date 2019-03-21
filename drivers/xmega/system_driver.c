#include "system_driver.h"

#include <avr/io.h>
#include "clksys_driver.h"

void System_Halt(void)
{
	CCPWrite(&SLEEP.CTRL, SLEEP_SMODE_PDOWN_gc | SLEEP_SEN_bm);
	__asm__ __volatile__ ("sleep");
}

void System_WakeUp(void)
{
	CCPWrite(&SLEEP.CTRL, 0);
}

void System_Reset(void)
{
	CCPWrite(&RST.CTRL, RST_SWRST_bm);
}

immutable_str System_LastResetCause(void)
{
	uint8_t status = RST.STATUS;
	if (status & RST_PORF_bm)
		return S("Power-on");
	if (status & RST_SRF_bm)
		return S("Software");
	if (status & RST_PDIRF_bm)
		return S("PDI");
	if (status & RST_EXTRF_bm)
		return S("External");
	if (status & RST_WDRF_bm)
		return S("Watchdog");
	if (status & RST_SDRF_bm)
		return S("Spike detection");
	if (status & RST_BORF_bm)
		return S("Brown-out");
	return NULL;
}

void System_ClearLastResetCause(void)
{
	RST.STATUS = RST_PORF_bm | RST_SRF_bm | RST_PDIRF_bm | RST_EXTRF_bm | RST_WDRF_bm | RST_SDRF_bm | RST_BORF_bm;
}

