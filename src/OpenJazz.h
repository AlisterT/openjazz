
/*
 *
 * OpenJazz.h
 *
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
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
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
#define CONFIG_FILE "openjazz.cfg"
#define LOGO_FILE   "openjazz.000"
#define LEVEL_FILE  "openjazz.tmp"

#ifdef UPPERCASE_FILENAMES
	#define F_MAINCHAR    "MAINCHAR.000"
	#define F_MENU        "MENU.000"
	#define F_PANEL       "PANEL.000"
	#define F_SOUNDS      "SOUNDS.000"
	#define F_BONUS       "BONUS.000"

	#define F_STARTUP_0SC "STARTUP.0SC"
	#define F_END_0SC     "END.0SC"

	#define F_MACRO       "MACRO.2"

	// File path prefixes
	#define F_BLOCKS      "BLOCKS"
	#define F_BONUSMAP    "BONUSMAP"
	#define F_LEVEL       "LEVEL"
	#define F_PLANET      "PLANET"
	#define F_SPRITES     "SPRITES"
#else
	#define F_MAINCHAR    "mainchar.000"
	#define F_MENU        "menu.000"
	#define F_PANEL       "panel.000"
	#define F_SOUNDS      "sounds.000"
	#define F_BONUS       "bonus.000"

	#define F_STARTUP_0SC "startup.0sc"
	#define F_END_0SC     "end.0sc"

	#define F_MACRO       "macro.2"

	// File path prefixes
	#define F_BLOCKS      "blocks"
	#define F_BONUSMAP    "bonusmap"
	#define F_LEVEL       "level"
	#define F_PLANET      "planet"
	#define F_SPRITES     "sprites"
#endif

// Standard string length
#define STRING_LENGTH 32

// Return values
#define E_DATA      -14
#define E_VERSION   -13
#define E_TIMEOUT   -12
#define E_N_OTHER   -11
#define E_N_CONNECT -10
#define E_N_ADDRESS -9
#define E_N_LISTEN  -8
#define E_N_BIND    -7
#define E_N_SOCKET  -6
#define E_DEMOTYPE  -5
#define E_FILE      -4
#define E_VIDEO     -3
#define E_UNUSED    -2
#define E_QUIT      -1
#define E_NONE       0
#define WON          1
#define LOST         2
#define JOYSTICKB    0x100
#define JOYSTICKANEG 0x200
#define JOYSTICKAPOS 0x300

// Time interval
#define T_FRAME 20


// Macros

// For fixed-point operations
#define FTOI(x) ((x) >> 10)
#define ITOF(x) ((x) << 10)
#define MUL(x, y) (((x) * (y)) >> 10)
#define DIV(x, y) (((x) << 10) / (y))


// Enum

enum LoopType {

	NORMAL_LOOP, TYPING_LOOP, SET_KEY_LOOP, SET_JOYSTICK_LOOP

};


// Datatype

typedef int fixed;


// Variable

// Time
EXTERN unsigned int globalTicks;


// Functions in main.cpp

EXTERN int loop (LoopType type);


// Functions in util.cpp

EXTERN bool   fileExists           (const char *fileName);
EXTERN char * createString         (const char *string);
EXTERN char * createString         (const char *first, const char *second);
EXTERN char * createFileName       (const char *type, int extension);
EXTERN char * createFileName       (const char *type, const char *extension);
EXTERN char * createFileName       (const char *type, int level, int extension);
EXTERN char * createEditableString (const char *string);
EXTERN void   log                  (const char *message);
EXTERN void   log                  (const char *message, const char *detail);
EXTERN void   log                  (const char *message, int number);
EXTERN void   logError             (const char *message, const char *detail);

#ifdef VERBOSE
#define LOG(x, y) log(x, y)
#else
#define LOG(x, y) x; y
#endif

#endif

