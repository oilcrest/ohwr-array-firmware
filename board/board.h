/**
 *  \file
 *
 *  \brief Board initialization interface.
 */
#ifndef _BOARD_H__
#define _BOARD_H__

#include "app_cfg.h" // board_id_t

/** \brief Initialize board features.
 */
void Board_Init(void);

extern board_id_t Board_id;

#endif // !_BOARD_H__
