/**
 *  \file
 *
 *  \brief System commands
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#include "astring.h"
#include "board.h"
#include "cli.h"
#include "clksys_getfreq.h"
#include "cmdarg.h"
#include "debug.h"
#include "sp_driver.h"
#include "stack_usage.h"
#include "sys_info.h"
#include "system_driver.h"
#include "ucos_ii.h"
#include "ui.h"
#include <stddef.h>

#include "TWI_master.h"

extern OS_STK mainTaskStack[MAIN_TASK_STACK_SIZE];

static result_t showBanner(void *pOut) {
  return CLI_TPRINTF("%S\n", SysInfo_banner);
}

static result_t showCpu(void *pOut, bool extended) {
  result_t res;
  uint32_t frq = CLKSYS_GetFrequency(CLKSYS_OUTPUT_CPU) / 1000000;
  res = CLI_TPRINTF("CPU: %S @ %" PRIu32 "MHz\n", SysInfo_GetCpuName(), frq);
  if (res != RESULT_OK || !extended)
    return res;

  return CLI_TPRINTFI(
      S("  Device ID: %02X%02X%02X\n"
        "Revision ID: %02X\n"
        "Production signature: Lot %02x%02x%02x%02x%02x%02x  Wafer %02X  X "
        "%02X%02X  Y %02X%02X\n"),
      // ID
      MCU.DEVID2, MCU.DEVID1, MCU.DEVID0, MCU.REVID,
      // LOT number
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM5)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM4)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM3)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM2)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM1)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, LOTNUM0)),
      // Wafer number
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, WAFNUM)),
      // coord X
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDX1)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDX0)),
      // coord Y
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDY1)),
      SP_ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, COORDY0)));
}

static result_t showBoardId(void *pOut) {
  return CLI_TPRINTF("Board ID: %08X\n", Board_id);
}

static result_t showBuild(void *pOut) {
  return CLI_TPRINTF("%S\n%S\n%S\n%S\n", SysInfo_build_rev, SysInfo_build_date,
                     SysInfo_build_user, SysInfo_build_machine);
}

static result_t showUptime(void *pOut) {
  uint32_t tim = OSTimeGet();
  uint16_t ms = (uint16_t)((tim % OS_TICKS_PER_SEC) * 1000 / OS_TICKS_PER_SEC);
  uint8_t s = (uint8_t)(tim / OS_TICKS_PER_SEC % 60);
  uint8_t m = (uint8_t)((tim / ((uint32_t)OS_TICKS_PER_SEC * 60)) % 60);
  uint8_t h = (uint8_t)(tim / ((uint32_t)OS_TICKS_PER_SEC * 3600));
  return CLI_TPRINTF("Uptime: "
                     "%02" PRIu8 "h "
                     "%02" PRIu8 "m "
                     "%02" PRIu8 "s "
                     "%03" PRIu16 "ms "
                     "(%" PRIu32 " ticks)\n",
                     h, m, s, ms, tim);
}

static result_t showDiag(void *pOut) {
  return CLI_TPRINTF("Peak stack usage:\n"
                     "Main  : %4u/%4u\n"
                     "UI    : %4u/%4u\n",
                     MAIN_TASK_STACK_SIZE - StackUsage_Peak(mainTaskStack),
                     MAIN_TASK_STACK_SIZE,
                     UI_TASK_STACK_SIZE - StackUsage_Peak(UITask_stack),
                     UI_TASK_STACK_SIZE);
}

DEFINE_COMMAND(ROOT_SYS, UPTIME, NULL, pObj, args, pOut) {
  return showUptime(pOut);
}

DEFINE_COMMAND(ROOT_SYS, BUILD, NULL, pObj, args, pOut) {
  return showBuild(pOut);
}

DEFINE_COMMAND(ROOT_SYS, INFO, NULL, pObj, args, pOut) {
  showBanner(pOut);
  showBuild(pOut);
  showBoardId(pOut);
  showCpu(pOut, false);
  showUptime(pOut);
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_SYS, BOARD, NULL, pObj, args, pOut) {
  return showBoardId(pOut);
}

DEFINE_COMMAND(ROOT_SYS, CPU, NULL, pObj, args, pOut) {
  return showCpu(pOut, true);
}

DEFINE_COMMAND(ROOT_SYS, DIAG, NULL, pObj, args, pOut) {
  return showDiag(pOut);
}

DEFINE_COMMAND(ROOT_SYS, HALT, NULL, pObj, args, pOut) {
  CLI_TPRINTF("Halting...");
  DPRINTF("Halting...");
  System_Halt();
  // just to keep compiler happy
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_SYS, REBOOT, NULL, pObj, args, pOut) {
  CLI_TPRINTF("Rebooting...");
  DPRINTF("Rebooting...");
  System_Reset();
  // just to keep compiler happy
  return RESULT_OK;
}

DEFINE_COMMAND_ARRAY(ROOT, SYS, INFO);
