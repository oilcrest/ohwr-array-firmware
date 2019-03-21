/**
 *  \file
 *
 *  \brief Initialization routines for uC/OS-II on TestBeam2 readout board.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#include "ucos_bsp.h"
#include "TC_driver.h"
#include "clksys_getfreq.h"
#include "debug.h"
#include "led.h"
#include "mt.h"
#include "pmic_driver.h"
#include "types.h"
#include "ucos_ii.h"
#include "ui.h"

/** \brief Initialize OS tick timer.
 *
 *  This function initializes OS tick timer and sets the tick rate.
 *  Tick handler is called on every timer overflow.
 *
 *  \see TCC0_OVF_vect
 *  \see OS_TICK_PER_SEC
 */
static void initTickTimer(void) {
  uint32_t clkPerFreq = CLKSYS_GetFrequency(CLKSYS_OUTPUT_CPU);

  // period = CLKCPU / (div * OS_TICKS_PER_SEC)
  // The equation actually performs rounding by multiplying by 2, adding 1 and
  // then dividing by 2 in integer math, this is equivalent to adding 0.5
  uint32_t period =
      (uint32_t)(((2 * clkPerFreq) + (256 * (uint32_t)OS_TICKS_PER_SEC)) /
                 ((256 * 2 * (uint32_t)OS_TICKS_PER_SEC)));

  TC0_Reset(&TCC0);
  TC0_ConfigClockSource(&TCC0, TC_CLKSEL_DIV256_gc);
  TC_SetPeriod(&TCC0, period);
  TC0_SetOverflowIntLevel(&TCC0, TC_OVFINTLVL_HI_gc);
  TC_ForceUpdate(&TCC0);
  PMIC_EnableHighLevel();
  TC_Restart(&TCC0);
}

uint8_t ui_is_display_active(void);

/** \brief Handle OS tick timer interrupt.
 */
void UCOS_TickHandler(void) {
  static uint8_t k;
  if ((++k) & 0x20) {
    if (ui_is_display_active())
      LED_On(LED_STATUS);
  } else
    LED_Off(LED_STATUS);
  OSTimeTick();
}

typedef struct StartTaskData_struct {
  void (*function)(void *);
  void *pData;
} StartTaskData;

static void startMainTask(void *pData) __attribute__((noreturn));
static void startMainTask(void *pData) {
  initTickTimer();

  DPRINTF("OK.\nuC/OS-II ver. %d.%2d\r\n", OSVersion() / 100,
          OSVersion() % 100);
  (*((StartTaskData *)pData)->function)(((StartTaskData *)pData)->pData);

  for (;;) {
  }
}

void UCOS_Main(void (*mainTask)(void *), void *pData, uint8_t *mainTaskStack,
               uint8_t prio) {
  DPRINTF("Initializing multitasking... ");
  PMIC_DisableLowLevel();
  PMIC_DisableMediumLevel();
  PMIC_DisableHighLevel();

  OSInit();

  StartTaskData taskData = {.function = mainTask, .pData = pData};

  OSTaskCreate(startMainTask, &taskData, mainTaskStack, prio);

  OSStart();

  for (;;) {
  }
}
