/**
 *  \file
 *
 *  \brief Board functions
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#include "board.h"
#include "clksys_driver.h"
#include "clksys_getfreq.h"
#include "config_file.h"
#include "dac_driver.h"
#include "debug.h"
#include "led.h"
#include "mt.h"
#include "sp_driver.h"
#include "swmatrix.h"
#include "system_driver.h"
#include <avr/io.h>
#include <stddef.h>

/** \brief Initialize the system clock and set the CPU frequency.
 *
 */
void initCpuClk(void) {
  /*  First try to enable external clock source until the timeout
   *  expires. Check if it's stable and set the external
   *  oscillator as the main clock source.
   */
  CLKSYS_Prescalers_Config(CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
  CLKSYS_Enable(OSC_RC32MEN_bm);
  while (CLKSYS_IsReady(OSC_RC32MRDY_bm) == 0) {
  }
  CLKSYS_Main_ClockSource_Select(CLK_SCLKSEL_RC32M_gc);
  /*  Whether it's working or not, we didn't switch
   *  to external clock, so return false.
   */
}

board_id_t Board_id;

/** \brief Set board id global variable.
 *
 *  The first intent was to read the board id from the voltage divider
 *  on ADC input. Since it's not mounted, the id is currently made
 *  of some unique AVR uC production signatures.
 */
static void setBoardId(void) {
  Board_id =
      (uint32_t)(
          SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDX1)) ^
          SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM3)))
          << 24 |
      (uint32_t)(
          SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDX0)) ^
          SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM2)))
          << 16 |
      (uint32_t)(
          SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDY1)) ^
          SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM1)))
          << 8 |
      (SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDY0)) ^
       SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM0)));
}

/** \brief Set calibration registers for ADC, DAC and temperature sensor.
 */
void initAnalog(void) {

  DAC_DualChannel_Enable(&DACA, DAC_REFSEL_INT1V_gc, 0, DAC_CONINTVAL_64CLK_gc,
                         DAC_REFRESH_16CLK_gc);
  DAC_DualChannel_Enable(&DACB, DAC_REFSEL_INT1V_gc, 0, DAC_CONINTVAL_64CLK_gc,
                         DAC_REFRESH_16CLK_gc);

  DACA.OFFSETCAL =
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, DACAOFFCAL));
  DACA.GAINCAL =
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, DACAGAINCAL));
  DACB.OFFSETCAL =
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, DACBOFFCAL));
  DACB.GAINCAL =
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, DACBGAINCAL));
}

void analog_supply_enable(void) {
  PORTF.DIRSET = 0x02;
  PORTF.OUTSET = 0x02;
}
void analog_supply_leds_enable(void) {
  PORTJ.DIRSET = 0x60;
  // VPOS -> high
  PORTJ.OUTSET = 0x40;
  // VNEG -> low
  PORTJ.OUTCLR = 0x20;
}

void analog_supply_leds_disable(void) {
  PORTJ.DIRSET = 0x60;
  // VPOS -> high
  PORTJ.OUTCLR = 0x40;
  // VNEG -> low
  PORTJ.OUTSET = 0x20;
}

void Board_Init(void) {
  // this should be later (after debug init works)
  // LED_Init();
  // LED_Off(LED_ALL);
  // LED_On(LED_STA1);

  System_WakeUp();

  LED_Init();
  LED_Off(LED_ALL);
  analog_supply_enable();
  analog_supply_leds_enable();
  //    LED_On(LED_STA);
  swmatrix_init();
  setBoardId();

  ConfigFile_Init();
  //    SCMonitor_Init();

  //  initAnalog();

  Debug_Init();
  DPRINTF("\n");

  //    uint16_t tempCal =
  //        SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, TEMPSENSE0))
  //        | (SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t,
  //        TEMPSENSE1)) << 8);
  //    DPRINTF("%d %04X\n", tempCal, tempCal);

  immutable_str lastResetCause = System_LastResetCause();
  System_ClearLastResetCause();
  {
    uint32_t bootCounter;
    ConfigFile_Load(CONFIGFILE_BOOT_COUNTER, &bootCounter, sizeof(bootCounter));
    ++bootCounter;
    if (lastResetCause)
      ConfigFile_Save(CONFIGFILE_BOOT_COUNTER, &bootCounter,
                      sizeof(bootCounter));
    DPRINTF("Booting (%" PRIu32 ")...\n", bootCounter);
  }

  DPRINTF("Last reset cause: %S\n",
          lastResetCause ? lastResetCause : S("Unknown"));

  initCpuClk();
  Debug_Init();
  DPRINTF("CPU clock frequency = %" PRIu32 " Hz\n", CLKSYS_GetCpuFrequency());
  DPRINTF("Peripheral clock frequency = %" PRIu32 " Hz\n",
          CLKSYS_GetFrequency(CLKSYS_OUTPUT_PER));
  //    LED_On(LED_STA2);
  //    LED_Off(LED_STA1);
}
