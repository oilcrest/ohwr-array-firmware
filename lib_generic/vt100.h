/**
 *  \file
 *
 *  \brief Support for VT100-compatible terminal on Stream.
 *
 *  Implementation of Terminal interface.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _VT100_H__
#define _VT100_H__

#include "stream.h"
#include "terminal.h"

/** \brief Initialize VT100 Terminal.
 */
void VT100_Init(Terminal *pTerminal, Stream *pStream);

#endif // !_VT100_H__
