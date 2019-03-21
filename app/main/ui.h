/**
 *  \file
 *
 *  \brief User Interface task header file.
 *
 *  \author Szymon Kulis, CERN, Geneva
 */

#ifndef _UI_TASK_H__
#define _UI_TASK_H__

#include "types.h"
#include "ucos_ii.h"

#ifndef UI_TASK_STACK_SIZE
#error "UI_TASK_STACK_SIZE not defined"
#endif

extern OS_STK UITask_stack[UI_TASK_STACK_SIZE];

typedef enum {
  UI_DISPLAY_OFF = 0,
  UI_DISPLAY_ON = 1,
  UI_DISPLAY_AUTO = 2,
} ui_display_t;

typedef enum {
  UI_REPRESENTATION_DEC = 0,
  UI_REPRESENTATION_HEX = 1,
  UI_REPRESENTATION_OCT = 2
} ui_representation_t;

typedef struct {
  ui_display_t display;
  uint8_t timeout;
  uint16_t timeElapsed;
  uint8_t autoDisplayState;
  uint16_t value;
  ui_representation_t representation;
} ui_cnf_t;

/**
 * \brief UI task entry point.
 */
void UiTask(void *pArg) __attribute__((noreturn));

void ui_set_display(ui_display_t displ);
ui_display_t ui_get_display();
void ui_set_timeout(uint8_t timeout);
uint8_t ui_get_timeout();

#endif // !_HWMON_TASK_H__
