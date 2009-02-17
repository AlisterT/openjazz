
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
#define F64  65536
#define F100 102400
#define F160 163840


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

// Return values
#define QUIT    -1 /* Only loop() and functions which call loop() can return
                      this */
#define SUCCESS  0
#define FAILURE  1
#define WON      2
#define LOST     3



// Datatype

typedef int fixed;


// Includes

#include "bonus.h"
#include "file.h"
#include "font.h"
#include "palette.h"
#include "player.h"
#include "planet.h"
#include "scene.h"
#include "events.h"
#include "bullet.h"
#include "level.h"
#include "menu.h"


// Variables

Extern struct {

	SDLKey key; // Keyboard key
	Uint8  state;

} keys[CONTROLS];

Extern struct {

	int    button; // Joystick button
	Uint8  state;

} buttons[CONTROLS];

Extern struct {

	int    axis; // Joystick axis
	int    direction; // Axis direction
	Uint8  state;

} axes[CONTROLS];

Extern struct {

	int   time; // The time from which the control will respond to being pressed
	Uint8 state;

} controls[CONTROLS];

Extern char *path; // Path to game data

Extern SDL_Surface *panel;
Extern SDL_Surface *panelAmmo[5];

Extern Font *font2;         /* Taken from .0FN file name */
Extern Font *fontbig;       /* Taken from .0FN file name */
Extern Font *fontiny;       /* Taken from .0FN file name */
Extern Font *fontmn1;       /* Taken from .0FN file name */
Extern Font *fontmn2;       /* Taken from .0FN file name */
Extern Font *panelBigFont;   /* Not a font file, found in PANEL.000 */
Extern Font *panelSmallFont; /* Not a font file, found in PANEL.000 */


Extern PaletteEffect *firstPE;
Extern SDL_Surface   *screen;
Extern SDL_Color     *currentPalette;
Extern SDL_Color      logicalPalette[256];
Extern int            screenW, screenH;
#ifndef FULLSCREEN_ONLY
Extern bool           fullscreen;
#endif
Extern int            mspf;

Extern Menu *menuInst;

Extern char *localPlayerName;

Extern Player *players;
Extern int     nPlayers;

Extern Level *levelInst;


// Functions in main.cpp

Extern void releaseControl (int control);
Extern void update         ();
Extern int  loop           ();


// Functions in palette.cpp

Extern void usePalette     (SDL_Color *palette);
Extern void scalePalette   (SDL_Surface *surface, fixed scale,
                            signed int offset);
Extern void restorePalette (SDL_Surface *surface);


// Functions in sound.cpp

Extern void openAudio  ();
Extern void closeAudio ();
Extern void playMusic  (char * fn);
Extern void stopMusic  ();
extern int  loadSounds (char * fn);
extern void freeSounds ();
Extern void playSound  (int sound);


// Functions in util.cpp

Extern int           fileExists         (char * fileName);
Extern SDL_Surface * createSurface      (unsigned char * pixels, int width,
                                         int height);
Extern SDL_Surface * createBlankSurface ();



