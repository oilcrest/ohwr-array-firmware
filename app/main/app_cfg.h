/**
 *  \file
 *
 *  \brief Application configuration.
 *
 *  \note The name of this file is given according to uC-OS/II
 *        recommendation, as ucos_ii.h includes this file.
 *        uC-OS/II developers recommend also keeping all application
 *        configuration in this file, it was adopted for compile time
 *        configuration of application and other components,
 *        such as library and drivers.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#ifndef _APP_CFG_H__
#define _APP_CFG_H__

#include "board_cfg.h"
#include "types.h"

typedef uint32_t board_id_t;

// task priorities
#define UI_TASK_PRIO 1
#define MAIN_TASK_PRIO (OS_LOWEST_PRIO - 3)
#define OS_TASK_TMR_PRIO (OS_LOWEST_PRIO - 2)

// stack sizes
#define UI_TASK_STACK_SIZE 1000
#define MAIN_TASK_STACK_SIZE 1000

// board and drivers features configuration

//#define XMEGA_USART_ENABLE_USARTE0
//#define XMEGA_USART_ENABLE_USARTE1
#define XMEGA_USART_ENABLE_USARTF0

// general configuration

#define ENABLE_ARGUMENT_CHECKS

// Console configuration

#define CONSOLE_FIFO_SIZE 32
#define CONSOLE_USART 0
#define CONSOLE_USART_BAUDRATE 115200

// Editor configuration

#define EDITOR_LINE_SIZE 77
#define EDITOR_BUFFER_SIZE 512

// Debug configuration

#define DEBUG_USART 0
#define DEBUG_USART_BAUDRATE 115200
// #define DISABLE_DEBUG

// EEPROM data addresses.
// Each section must be aligned to EEPROM page boundary (0x20 bytes)

#define CONFIGFILE_BOOT_COUNTER 0x0000

#define CONFIGFILE_HWMON_TASK_AUTO 0x0040

#endif // !_APP_CFG_H__
