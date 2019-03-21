/**
 *  \file
 *
 *  \brief Switching matrix main program
 *
 *  \author Szymon Kulis, CERN, Geneva
 */

#include "board.h"
#include "cli.h"
#include "debug.h"
#include "fifo.h"
#include "led.h"
#include "serialstream.h"
#include "sp_driver.h"
#include "stack_usage.h"
#include "swmatrix.h"
#include "ucos_bsp.h"
#include "ucos_ii.h"
#include "ui.h"
#include <stddef.h>

OS_STK mainTaskStack[MAIN_TASK_STACK_SIZE];

// static void waitms(uint16_t ms)
//{
//	OSTimeDlyHMSM(0, 0, ms / 1000, ms % 1000);
//}

static void mainTask(void *pArg) __attribute__((noreturn));
static void mainTask(void *pArg) {
  (void)pArg;

  // result_t res;

  DPRINTF("Switchin Matrix init ... ");
  if (swmatrix_init() == RESULT_OK)
    DPRINTF("OK.\n");
  else
    DPRINTF("FAILED!\n");
  OSTimeDlyHMSM(0, 0, 0, 10);

  // initialize stream for DAQ output
  // then start daq and tx tasks

  uint8_t consoleInBuffer[CONSOLE_FIFO_SIZE];
  ByteFifo consoleInFifo;
  ByteFifo_Init(&consoleInFifo, consoleInBuffer, CONSOLE_FIFO_SIZE);
  static Stream consoleStream;
  Serial_Init(CONSOLE_USART, CONSOLE_USART_BAUDRATE, &consoleInFifo, NULL,
              SERIAL_USE_TX_DMA);
  SerialStream_Init(&consoleStream, CONSOLE_USART, STREAM_MODE_TEXT);

  DPRINTF("Starting UI task ... ");
  OSTaskCreate(&UiTask, 0, &UITask_stack[UI_TASK_STACK_SIZE - 1], UI_TASK_PRIO);

  // open serial port and initialize stream for CLI
  // then start CLI task
  DPRINTF("Starting CLI... ");
  CLI_Run(&consoleStream);
}

int main(void) {
  StackUsage_Fill(mainTaskStack, MAIN_TASK_STACK_SIZE);
  Board_Init();
  UCOS_Main(mainTask, 0, &mainTaskStack[MAIN_TASK_STACK_SIZE - 1],
            MAIN_TASK_PRIO);
  LED_Off(LED_ALL);
  LED_On(LED_STATUS);
  for (;;) {
  }
}
