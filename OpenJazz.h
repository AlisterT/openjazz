
/*
 *
 * OpenJazz.h
 *
 * Created on the 23rd of August 2005
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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


#include <SDL/SDL.h>


#ifndef Extern
	#define Extern extern
#endif

/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


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

// File names
#define CONFIG_FILE "openjazz.cfg"
#define LOGO_FILE   "openjazz.000"
#define LEVEL_FILE  "openjazz.tmp"

#ifdef UPPERCASE_FILENAMES
	#define F_MAINCHAR "MAINCHAR.000"
	#define F_MENU     "MENU.000"
	#define F_PANEL    "PANEL.000"

	// File name formats
	#define F_BLOCKS   "BLOCKS.%3s"
	#define F_LEVEL    "LEVEL%1i.%03i"
	#define F_PLANET   "PLANET.%3s"
	#define F_SPRITES  "SPRITES.%03i"
#else
	#define F_MAINCHAR "mainchar.000"
	#define F_MENU     "menu.000"
	#define F_PANEL    "panel.000"

	// File name formats
	#define F_BLOCKS   "blocks.%3s"
	#define F_LEVEL    "level%1i.%03i"
	#define F_PLANET   "planet.%3s"
	#define F_SPRITES  "sprites.%03i"
#endif

// Indexes for the keys / buttons / axes player controls arrays
#define C_UP      0
#define C_DOWN    1
#define C_LEFT    2
#define C_RIGHT   3
#define C_JUMP    4
#define C_FIRE    5
#define C_CHANGE  6 /* Change weapon */
#define C_ENTER   7
#define C_ESCAPE  8
#define C_STATS   9
#define C_PAUSE  10
// Size of those arrays
#define CONTROLS 11

// Standard string length
#define STRING_LENGTH 32

// Loop return type
#define NORMAL_LOOP   0
#define KEY_LOOP      1
#define JOYSTICK_LOOP 2

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

// Time intervals
#define T_FRAME 20
#define T_KEY   500

// Black palette index
#define BLACK      31


// Datatype

typedef int fixed;


// Variables

Extern struct {

	int           key; // Keyboard key
	unsigned char state;

} keys[CONTROLS];

Extern struct {

	int           button; // Joystick button
	unsigned char state;

} buttons[CONTROLS];

Extern struct {

	int           axis; // Joystick axis
	int           direction; // Axis direction
	unsigned char state;

} axes[CONTROLS];

Extern struct {

	int  time; // The time from which the control will respond to being pressed
	bool state;

} controls[CONTROLS];


// Panel
Extern SDL_Surface *panel;
Extern SDL_Surface *panelAmmo[5];

// Graphics
Extern SDL_Surface *screen;
Extern int          viewW, viewH, screenW, screenH;
#ifndef FULLSCREEN_ONLY
Extern bool         fullscreen;
#endif
Extern int          mspf;

// Configuration data
Extern char          *characterName;
Extern unsigned char  characterCols[4];
Extern char          *netAddress;



// Functions in main.cpp

Extern void releaseControl (int control);
Extern int  loop           (int type);


// Functions in util.cpp

Extern bool          fileExists         (char *fileName);
Extern SDL_Surface * createSurface      (unsigned char *pixels, int width,
                                         int height);
Extern char        * cloneString        (char *string);
Extern void          clearScreen        (int index);
Extern void          drawRect           (int x, int y, int width, int height,
	int index);

#endif

