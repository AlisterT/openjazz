
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


// Constants

// Numbers in -10 exponent fixed point
#define FE   128
#define FQ   256
#define FH   512
#define F1   1024
#define F2   2048
#define F4   4096
#define F8   8192
#define F10  10240
#define F12  12288
#define F16  16384
#define F20  20480
#define F24  24576
#define F32  32768
#define F36  36864
#define F40  40960
#define F80  81920
#define F64  65536
#define F100 102400
#define F160 163840
#define F192 196608

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
#define E_RETURN       -(0x02)
#define E_QUIT         -(0x01)

#define E_NONE       0


// Macros

// For fixed-point operations
#define FTOI(x) ((x) >> 10) ///< Fixed to Int
#define ITOF(x) ((x) << 10) ///< Int to Fixed
#define MUL(x, y) (((x) * (y)) >> 10) ///< multiplication
#define DIV(x, y) (((x) << 10) / (y)) ///< division


// Datatype

typedef int fixed; ///< Custom fixed-point data type

#endif

