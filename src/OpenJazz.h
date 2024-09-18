
/**
 *
 * @file OpenJazz.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 31st January 2006: Created player.h from parts of OpenJazz.h
 * - 3rd February 2009: Created menu.h from parts of OpenJazz.h
 * - 3rd February 2009: Created file.h from parts of OpenJazz.h
 * - 3rd February 2009: Created font.h from parts of OpenJazz.h
 * - 4th February 2009: Created palette.h from parts of OpenJazz.h
 * - 2nd March 2009: Created network.h from parts of OpenJazz.h
 * - 2nd June 2009: Created sound.h from parts of OpenJazz.h
 * - 3rd June 2009: Created network.h from parts of OpenJazz.h
 * - 13th July 2009: Created controls.h from parts of OpenJazz.h
 * - 13th July 2009: Created graphics.h from parts of OpenJazz.h
 * - 30th April 2010: Created util.h from parts of OpenJazz.h
 * - 30th April 2010: Created loop.h from parts of OpenJazz.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _OPENJAZZ_H
#define _OPENJAZZ_H

#ifndef EXTERN
	#define EXTERN extern
#endif

#include "types.h"

// Constants

// Standard string length
#define STRING_LENGTH 32

// Return values
#define E_N_OTHER      -(0x26)
#define E_N_DISCONNECT -(0x25)
#define E_N_CONNECT    -(0x24)
#define E_N_ADDRESS    -(0x23)
#define E_N_LISTEN     -(0x22)
#define E_N_BIND       -(0x21)
#define E_N_SOCKET     -(0x20)
#define E_DATA         -(0x15)
#define E_VERSION      -(0x14)
#define E_TIMEOUT      -(0x13)
#define E_DEMOTYPE     -(0x12)
#define E_FILE         -(0x11)
#define E_VIDEO        -(0x10)
#define E_LOAD0        -(0x0F)
#define E_LOAD1        -(0x0E)
#define E_LOAD2        -(0x0D)
#define E_LOAD3        -(0x0C)
#define E_RETURN       -(0x02)
#define E_QUIT         -(0x01)

#define E_NONE       0

#define MAX_PALETTE_COLORS 256

#endif
