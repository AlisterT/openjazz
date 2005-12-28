
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

#ifdef Main
 #define Extern
#else
 #define Extern extern
#endif


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


// Constants

// Types of palette effect
#define PE_FADE   0 /* Fades to black, then remains black */
#define PE_ROTATE 1 /* Cyclical colour animation */
#define PE_SKY    2 /* Transfers the appropriate section of the background
                       palette to the main palette */
#define PE_1D     3 /* Diagonal lines parallaxing background */
#define PE_2D     4 /* Real parallaxing background */
#define PE_WATER  5 /* After a threshold, fades to black, then remains black */

// Indexes for the key array
#define K_UP       1
#define K_DOWN     2
#define K_LEFT     3
#define K_RIGHT    4
#define K_JUMP     5
#define K_FIRE     6
#define K_CHANGE   7 /* Change weapon */

// Indexes for elements of the event set
/* Plagiarised shamelessly from J1CS / JCS94 / Project *that* / Whatever
 * ...Except of course it carries on the fine JCF tradition of changing the
 * spelling of words such as "behavior" */
#define E_DIFFICULTY     0
#define E_BYTE2          1
#define E_REFLECTION     2
#define E_BYTE4          3
#define E_BEHAVIOR       4
#define E_BEHAVIOUR      4
#define E_LEFTANIM       5
#define E_RIGHTANIM      6
#define E_BYTE8          7
#define E_MAGNITUDE      8
#define E_HITSTOKILL     9
#define E_MODIFIER       10
#define E_ADDEDSCORE     11
#define E_BYTE13         12
#define E_ENEMYSHOT      13
#define E_ENEMYSHOTSP    14
#define E_MOVEMENTSP     15
#define E_BYTE17         16
#define E_ANIMSP         17
#define E_BYTE19         18
#define E_BYTE20         19
#define E_BYTE21         20
#define E_BYTE22         21
#define E_MULTIPURPOSE   22
#define E_YAXIS          23
#define E_BRIDGELENGTH   24
#define E_MEDIVOBLEN     25
#define E_MEDIVOBDIRECT  26
#define E_BYTE28         27
#define E_FINISHANIM     28 /* also 29 */
#define E_FINISHSHOTANIM 30 /* also 31 */

// Player animations
#define PA_LWALK    0
#define PA_RWALK    1
#define PA_LJUMP    2
#define PA_RJUMP    3
#define PA_LSPIN    4
#define PA_RSPIN    5
#define PA_LSHOOT   6
#define PA_RSHOOT   7
#define PA_LCROUCH  8
#define PA_RCROUCH  9
#define PA_LFALL    10
#define PA_RFALL    11
#define PA_LHURT    12
#define PA_RHURT    13
#define PA_LLEAN    14
#define PA_RLEAN    15
#define PA_LBOARD   16
#define PA_RBOARD   17
#define PA_LSTAND   18
#define PA_RSTAND   19
#define PA_LEAT     20
#define PA_REAT     21
#define PA_LEDGE    22
#define PA_REDGE    23
#define PA_LOOKUP   24
#define PA_LOOKDOWN 25
#define PA_LSWIM    26
#define PA_RSWIM    27
#define PA_LRUN     28
#define PA_RRUN     29
#define PA_LDIE     30
#define PA_RDIE     31
#define PA_LSTOP    32
#define PA_RSTOP    33
#define PA_LHALT    34 /* Yeah, I was wondering the same thing... */
#define PA_RHALT    35
#define PA_RSPRING  36
#define PA_LSPRING  37 /* Surely these are the wrong way round? */

// Player speeds
#define PS_WALK  300
#define PS_RUN   325
#define PS_FALL  350
#define PS_JUMP -350

// General
#define LW        256 /* Level width */
#define LH         64 /* Level height */
#define BLACK      31
#define EVENTS    127
#define ELENGTH    32 /* Length of events, in bytes */
#define SPRITES   256
#define ANIMS     125
#define PANIMS     38 /* Number of player animations. Is probably higher. */
#define TW         32 /* Tile width */
#define TH         32 /* Tile height */
#define KEYS        8 /* As in keyboard keys */
#define SKEY      254 /* As in sprite colour key */
#define TKEY      127 /* As in tileset colour key */
#define WALKTIME  500 /* Time before a walk becomes a run */
#define JUMPTIME  40000 /* Divide by 100 when springs etc. all work */
#define LEANTIME 1000 /* Time before walking into a wall becomes a lean */

// Return values that should be acted upon
#define CONTINUE  0
#define RETURN    1
#define QUIT     -1

// Macro to do that acting
// r = the return value
// c = cleanup function
#define DORETURN(r, c)                  \
                        switch (r) {    \
                          case QUIT:    \
                            c           \
                            quit();     \
                          case RETURN:  \
                            c           \
                            return;     \
                        }



// Datatypes


typedef struct _paletteEffect paletteEffect;

struct _paletteEffect {

  paletteEffect *next;     // Next effect to use
  unsigned char  first;    // The first palette index affected by the effect
  unsigned char  amount;   /* The number of (consecutive) palette indices
                              affected by the effect */
  int            type;     /* Type of effect, see PE constants */
  float          speed;    /* When type is:
                              PE_FADE - Number of seconds the fade lasts
                              PE_ROTATE - Rotations per second
                              PE_SKY - Relative Y speed - as in Jazz 2
                              PE_1D - Relative X & Y speed - as in Jazz 2
                              PE_2D - Relative X & Y speed - as in Jazz 2
                              PE_WATER - Number of pixels between water surface
                                         and total darkness
                             */
  float          position; /* When type is:
                              PE_FADE - Brightness of palette in 0-1 range
                              PE_ROTATE - Number of colours rotated
                              PE_SKY - Position taken from player's viewport
                                       coordinates, in pixels
                              PE_1D, PE_2D - Position taken from player's
                                             viewport coordinates as short ints
                                             (in pixels), then packed into the
                                             float's 4 bytes
                              PE_WATER - Position of the water surface in pixels
                              */

};


typedef struct {

  SDL_Surface *pixels;
  unsigned char w;
  unsigned char h; // Dimensions of the letters
  char map[128]; // Maps ASCII values to letter positions

} font;


typedef struct {

  unsigned char tile;   // Indexes the tile set
  unsigned char bg;     /* 0 = Normal background
                           1 = Black background */
  unsigned char event;  // Indexes the event set

  // The following require more research into events
  unsigned char x, y;   /* Actual coordinates of the event assigned to this
                           tile, if applicable */
  unsigned char health; // Health of the event, if applicable

} gridElement;

typedef struct {

  SDL_Surface   *sprites[19]; // Pointers to members of the sprite set
  signed char    y[19];       // Vertical offsets (for pickup wobble effect)
  unsigned char  frames;      // Number of frames

} anim;

typedef struct {

    char  anims[PANIMS];
    int   anim;
    float x, y;
    float viewX, viewY;
    int   viewW, viewH;
    int   walkStart;
    int   jumpStart;
    int   score;
    int   energy;
    float energyBar;
    int   lives;
    int   facing; /* 0 = left, 1 = right. The order is needed for animation. */
    int   rising; /* 0 = not rising, 1 = jumping, 2 = somersaulting,
                     3 = red spring, 4 = green spring, 5 = blue spring */

} player;


// Variables

Extern struct {

  // It's pretty safe to assume there's one of these, but that's about it
  SDL_Surface *pixels;

} cutscene;


Extern struct {

  SDLKey key;
  Uint8  state;

} keys[8];


Extern player players[1]; // To do: Replace for multiplayer

Extern char *path; // Path to game data

Extern SDL_Surface *panel;

Extern font font2;          /* Taken from .0FN file name */
Extern font fontbig;        /* Taken from .0FN file name */
Extern font fontiny;        /* Taken from .0FN file name */
Extern font fontmn1;        /* Taken from .0FN file name */
Extern font fontmn2;        /* Taken from .0FN file name */
Extern font panelBigFont;   /* Not a font file, found in the PANEL.000 */
Extern font panelSmallFont; /* Not a font file, found in the PANEL.000 */

Extern gridElement    grid[LH][LW]; // All levels are the same size
Extern unsigned char  eventSet[EVENTS][ELENGTH]; // Not all used
Extern SDL_Surface   *spriteSet[SPRITES]; // 208 of which are in mainchar.000
Extern anim           animSet[ANIMS];
Extern SDL_Surface   *tileSet;
Extern char           mask[240][64]; // At most 240 tiles, all with 8 * 8 masks

Extern SDL_Color      realPalette[256];
Extern SDL_Color      bgPalette[255];
Extern paletteEffect *firstPE;
Extern paletteEffect *bgPE;
Extern int            bgScale;

Extern SDL_Surface *screen;
Extern int   screenW, screenH, fullscreen;
Extern float fps;
Extern float spf;

Extern int world, level, nextworld, nextlevel;
Extern int difficulty;


// Functions in bonus.c

Extern int  loadBonus (char * fn);
Extern void freeBonus (void);
Extern void bonusLoop (void);


// Functions in font.c

Extern font * loadFont   (char * fn);
Extern void   freeFont   (font * f);
Extern void   showString (char * s, int x, int y, font * f);
Extern void   showNumber (int n, int x, int y, font * f);


// Functions in level.c

Extern int             loadSprites   (char * fn);
Extern void            freeSprites   (void);
Extern int             loadTiles     (char * fn);
Extern int             loadLevel     (char * fn);
Extern int             loadNextLevel (void);
Extern int             loadMacro     (char * fn);
Extern void            freeLevel     (void);
Extern int             checkMask     (int x, int y);
Extern void            processEvent  (int x, int y, int frame);
Extern void            levelLoop     (void);


// Functions in main.c

Extern FILE          * fopenFromPath  (char * fn);
Extern unsigned char * loadRLE        (FILE * f, int size);
Extern void            skipRLE        (FILE * f);
Extern SDL_Surface   * createSurface  (unsigned char * pixels, int width,
                                       int height);
Extern int             loadMain       (void);
Extern void            updatePalettes (void);
Extern int             loop           (void);
Extern void            quit           (void);


// Functions in menu.c

Extern int  loadMenu (void);
Extern void freeMenu (void);
Extern void menuLoop (void);


// Functions in scene.c

Extern int  loadPlanet (char * fn);
Extern void freePlanet (void);
Extern void planetLoop (void);


// Functions in scene.c

Extern int  loadScene (char * fn);
Extern void freeScene (void);
Extern void sceneLoop (void);


// Functions in sound.c

Extern int  loadMusic  (char * fn);
Extern void freeMusic  (void);
Extern int  loadSounds (char *fn);
Extern void freeSounds (void);
Extern void playSound  (int sound);



