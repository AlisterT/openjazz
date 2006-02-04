
/*
 *
 * OpenJazz.h
 * Created on the 23rd of August 2005
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include <stdlib.h>
#include <SDL/SDL.h>

#ifndef Extern
  #define Extern extern
#endif

#if defined(_arch_dreamcast) || defined(__GP2X__) || defined(GP32)
  #define FULLSCREEN_ONLY
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
#define F6   6144
#define F8   8192
#define F10  10240
#define F12  12288
#define F16  16384
#define F19  19456
#define F20  20480
#define F24  24576
#define F26  26624
#define F30  30720
#define F32  32768
#define F36  36864
#define F40  40960
#define F64  65536
#define F160 163840
#define F256 262144

// Types of palette effect
#define PE_FADE   0 /* Fades to black, then remains black */
#define PE_ROTATE 1 /* Cyclical colour animation */
#define PE_SKY    2 /* Transfers the appropriate section of the background
                       palette to the main palette */
#define PE_1D     3 /* Diagonal lines parallaxing background */
#define PE_2D     4 /* Real parallaxing background */
#define PE_WATER  5 /* After a threshold, fades to black, then remains black */

// Indexes for the keys / buttons / axes / player.controls arrays
#define C_UP     1
#define C_DOWN   2
#define C_LEFT   3
#define C_RIGHT  4
#define C_JUMP   5
#define C_FIRE   6
#define C_CHANGE 7 /* Change weapon */
#define C_ENTER  8
#define C_ESCAPE 9
// Size of those arrays
#define CONTROLS 10

// Return values
#define SUCCESS  0
#define FAILURE  1
#define QUIT    -1 /* Only loop() and functions which call loop() can return
                      this */



// Datatypes

typedef int fixed;

typedef struct _paletteEffect paletteEffect;

struct _paletteEffect {

  paletteEffect *next;     // Next effect to use
  unsigned char  first;    // The first palette index affected by the effect
  unsigned char  amount;   /* The number of (consecutive) palette indices
                              affected by the effect */
  unsigned char  type;     /* Type of effect, see PE constants */
  fixed          speed;    /* When type is:
                              PE_FADE - Number of seconds the fade lasts
                              PE_ROTATE - Rotations per second
                              PE_SKY - Relative Y speed - as in Jazz 2
                              PE_1D - Relative X & Y speed - as in Jazz 2
                              PE_2D - Relative X & Y speed - as in Jazz 2
                              PE_WATER - Number of pixels between water surface
                                         and total darkness
                             */
  fixed          position; /* When type is:
                              PE_FADE - Brightness of palette in 0-1 range
                              PE_ROTATE - Number of colours rotated
                              PE_SKY - Position taken from player's viewport
                                       coordinates, in pixels
                              PE_1D, PE_2D - Position taken from player's
                                             viewport coordinates as short ints
                                             (in pixels), then packed into the
                                             fixed's 4 bytes
                              PE_WATER - Position of the water surface in pixels
                              */

};


typedef struct {

  SDL_Surface   *pixels;
  unsigned char *w;
  unsigned char  h; // Dimensions of the letters
  char           map[128]; // Maps ASCII values to letter positions

} font;


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

Extern font *font2;         /* Taken from .0FN file name */
Extern font *fontbig;       /* Taken from .0FN file name */
Extern font *fontiny;       /* Taken from .0FN file name */
Extern font *fontmn1;       /* Taken from .0FN file name */
Extern font *fontmn2;       /* Taken from .0FN file name */
Extern font panelBigFont;   /* Not a font file, found in PANEL.000 */
Extern font panelSmallFont; /* Not a font file, found in PANEL.000 */

Extern paletteEffect *firstPE;
Extern int            bgScale;

Extern SDL_Surface *screen;
Extern SDL_Color   *currentPalette;
Extern SDL_Color    logicalPalette[255];
Extern int          screenW, screenH;
#ifndef FULLSCREEN_ONLY
Extern int          fullscreen;
#endif
Extern float        fps;
Extern int          mspf;

Extern char          *currentLevel, *nextLevel;
Extern int            level, world;
Extern unsigned char  difficulty;
Extern SDL_Color      skyPalette[255];


// Function in bonus.c

Extern int     runBonus  (char * fn);


// Functions in font.c

Extern void   showString (char * s, int x, int y, font * f);
Extern void   showNumber (int n, int x, int y, font * f);


// Function in level.c

Extern int  runLevel      (char * fn);


// Functions in main.c

Extern FILE          * fopenFromPath      (char * fileName);
Extern int             fileExists         (char * fileName);
Extern unsigned char * loadRLE            (FILE * f, int size);
Extern void            skipRLE            (FILE * f);
Extern char          * loadString         (FILE * f);
Extern SDL_Surface   * createSurface      (unsigned char * pixels, int width,
                                           int height);
Extern SDL_Surface   * createBlankSurface (void);
#define loadSurface(file, width, height) \
  createSurface(loadRLE(file, (width) * (height)), width, height)
Extern void            loadPalette        (SDL_Color *palette, FILE *f);
Extern void            usePalette         (SDL_Color *palette);
Extern void            scalePalette       (SDL_Surface *surface, fixed scale,
                                           signed int offset);
Extern void            restorePalette     (SDL_Surface *surface);
Extern void            releaseControl     (int control);
Extern int             loop               (void);


// Function in menu.c

Extern int  runMenu  (void);


// Function in planet.c

Extern int  runPlanet  (char * fn);


// Function in scene.c

Extern int  runScene  (char * fn);


// Functions in sound.c

Extern int  loadMusic  (char * fn);
Extern void freeMusic  (void);
Extern void playSound  (int sound);



