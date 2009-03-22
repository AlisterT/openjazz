
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
#define E_S_OTHER   -11
#define E_S_CONNECT -10
#define E_S_ADDRESS -9
#define E_S_LISTEN  -8
#define E_S_BIND    -7
#define E_S_SOCKET  -6
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


// Datatype

typedef int fixed;


// Includes

#include "bonus.h"
#include "file.h"
#include "font.h"
#include "palette.h"
#include "bird.h"
#include "player.h"
#include "planet.h"
#include "scene.h"
#include "events.h"
#include "bullet.h"
#include "game.h"
#include "sprite.h"
#include "level.h"
#include "menu.h"


// Defaults
#define NET_ADDRESS "192.168.0.1"
#define NET_PORT    10052
#define CHAR_NAME   "jazz"
#define CHAR_FUR    PC_LGREEN
#define CHAR_BAND   PC_RED
#define CHAR_GUN    PC_BLUE
#define CHAR_WBAND  PC_ORANGE


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


// Path to game data
Extern char *path;

// Panel
Extern SDL_Surface *panel;
Extern SDL_Surface *panelAmmo[5];

// Fonts
Extern Font *font2;          /* Taken from .0FN file name */
Extern Font *fontbig;        /* Taken from .0FN file name */
Extern Font *fontiny;        /* Taken from .0FN file name */
Extern Font *fontmn1;        /* Taken from .0FN file name */
Extern Font *fontmn2;        /* Taken from .0FN file name */
Extern Font *panelBigFont;   /* Not a font file, found in PANEL.000 */
Extern Font *panelSmallFont; /* Not a font file, found in PANEL.000 */

// Graphics
Extern PaletteEffect *firstPE;
Extern SDL_Surface   *screen;
Extern SDL_Color     *currentPalette;
Extern SDL_Color      logicalPalette[256];
Extern fixed          viewX, viewY;
Extern int            viewW, viewH, screenW, screenH;
#ifndef FULLSCREEN_ONLY
Extern bool           fullscreen;
#endif
Extern int            mspf;

// Menu & gameplay
Extern Menu          *menu;
Extern Game          *game;
Extern Player        *players;
Extern Player        *localPlayer;
Extern int            nPlayers;
Extern Level         *level;
Extern unsigned char  checkX, checkY;

// Configuration data
Extern char          *characterName;
Extern unsigned char  characterCols[4];
Extern char          *netAddress;



// Functions in main.cpp

Extern void releaseControl (int control);
Extern int  loop           (int type);


// Functions in palette.cpp

Extern void usePalette     (SDL_Color *palette);
Extern void restorePalette (SDL_Surface *surface);


// Functions in sound.cpp

Extern void openAudio  ();
Extern void closeAudio ();
Extern void playMusic  (char *fn);
Extern void stopMusic  ();
extern int  loadSounds (char *fn);
extern void freeSounds ();
Extern void playSound  (int sound);


// Functions in util.cpp

Extern bool          fileExists         (char *fileName);
Extern SDL_Surface * createSurface      (unsigned char *pixels, int width,
                                         int height);
Extern char        * cloneString        (char *string);
Extern void          clearScreen        (int index);
Extern void          drawRect           (int x, int y, int width, int height,
	int index);



