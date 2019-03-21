/**
 *  \file
 *
 *  \brief Configuration of TestBeam2 board features, not dependent
 *         on application.
 *
 *  \note Don't include this file directly in drivers or so.
 *        Include it once in app_cfg.h and then use only the latter.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#ifndef _BOARD_CFG_H__
#define _BOARD_CFG_H__

/** \brief Frequency of the external oscillator - provided by FPGA
 */
#define CLKSYS_XOSC_FREQUENCY 24000000

// device addresses

#define TB2DC_DAQ_TIME_ADDR 0x6020

#endif // !_BOARD_CFG_H__
