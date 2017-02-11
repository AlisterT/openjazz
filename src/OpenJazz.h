
/**
 *
 * @file OpenJazz.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 31st January 2006: Created level.h from parts of OpenJazz.h
 * 31st January 2006: Created player.h from parts of OpenJazz.h
 * 3rd February 2009: Created menu.h from parts of OpenJazz.h
 * 3rd February 2009: Created file.h from parts of OpenJazz.h
 * 3rd February 2009: Created font.h from parts of OpenJazz.h
 * 4th February 2009: Created palette.h from parts of OpenJazz.h
 * 2nd March 2009: Created network.h from parts of OpenJazz.h
 * 2nd June 2009: Created sound.h from parts of OpenJazz.h
 * 3rd June 2009: Created network.h from parts of OpenJazz.h
 * 13th July 2009: Created controls.h from parts of OpenJazz.h
 * 13th July 2009: Created graphics.h from parts of OpenJazz.h
 * 30th April 2010: Created util.h from parts of OpenJazz.h
 * 30th April 2010: Created loop.h from parts of OpenJazz.h
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

// Files
#ifdef __SYMBIAN32__
#ifdef UIQ3
#define CONFIG_FILE "c:\\shared\\openjazz\\openjazz.cfg"
#else
#define CONFIG_FILE "c:\\data\\openjazz\\openjazz.cfg"
#endif
#else
#define CONFIG_FILE "openjazz.cfg"
#endif
#define LOGO_FILE   "openjazz.000"
#define LEVEL_FILE  "openjazz.tmp"

#define F_FONTS        "FONTS.000"
#define F_MAINCHAR     "MAINCHAR.000"
#define F_MENU         "MENU.000"
#define F_PANEL        "PANEL.000"
#define F_SOUNDS       "SOUNDS.000"
#define F_BONUS        "BONUS.000"
#define F_ANIMS_J2A    "ANIMS.J2A"

#define F_BONUS_0SC    "BONUS.0SC"
#define F_END_0SC      "END.0SC"
#define F_INSTRUCT_0SC "INSTRUCT.0SC"
#define F_ORDER_0SC    "ORDER.0SC"
#define F_STARTUP_0SC  "STARTUP.0SC"

#define F_MACRO        "MACRO.1"

#define F_FONT2_0FN    "FONT2.0FN"
#define F_FONTBIG_0FN  "FONTBIG.0FN"
#define F_FONTINY_0FN  "FONTINY.0FN"
#define F_FONTMN1_0FN  "FONTMN1.0FN"
#define F_FONTMN2_0FN  "FONTMN2.0FN"

// File path prefixes
#define F_BLOCKS       "BLOCKS"
#define F_BONUSMAP     "BONUSMAP"
#define F_LEVEL        "LEVEL"
#define F_PLANET       "PLANET"
#define F_SPRITES      "SPRITES"


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

#define WON          1
#define LOST         2
#define JOYSTICKB    0x100
#define JOYSTICKANEG 0x200
#define JOYSTICKAPOS 0x300

// Time intervals
#define T_FRAME        20
#define T_ACTIVE_FRAME 4
#define T_STEP         16


// Macros

// For fixed-point operations
#define FTOI(x) ((x) >> 10)
#define ITOF(x) ((x) << 10)
#define MUL(x, y) (((x) * (y)) >> 10)
#define DIV(x, y) (((x) << 10) / (y))


// Datatype

typedef int fixed;


// Variable

EXTERN unsigned int globalTicks;


#endif

