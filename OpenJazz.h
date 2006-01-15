
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

// Player facing
#define PF_LEFT 0
#define PF_RIGHT 1

// Player reactions
#define PR_NONE       0
#define PR_HURT       1
#define PR_KILLED     2
#define PR_INVINCIBLE 3
#define PR_WON        4

// Player speeds
#define PS_WALK  300
#define PS_RUN   325
#define PS_FALL  350
#define PS_JUMP -350

// Bullet types
#define B_BLASTER  41
#define B_LTOASTER 42
#define B_RTOASTER 51
#define B_LMISSILE 52
#define B_RMISSILE 43
#define B_BOUNCER  49
#define B_TNT      130

// General
#define LW        256 /* Level width */
#define LH         64 /* Level height */
#define BLACK      31
#define EVENTS    127
#define ELENGTH    32 /* Length of events, in bytes */
#define ANIMS     128
#define PANIMS     38 /* Number of player animations. Is probably higher. */
#define TW         32 /* Tile width */
#define TH         32 /* Tile height */
#define KEYS        8 /* As in keyboard keys */
#define SKEY      254 /* As in sprite colour key */
#define TKEY      127 /* As in tileset colour key */
#define HURTTIME 1000
#define EOLTIME  4000

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
  unsigned char  type;     /* Type of effect, see PE constants */
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

  SDL_Surface   *pixels;
  unsigned char *w;
  unsigned char  h; // Dimensions of the letters
  char           map[128]; // Maps ASCII values to letter positions

} font;


typedef struct {

  unsigned char tile;  // Indexes the tile set
  unsigned char bg;    /* 0 = Normal background
                          1 = Black background */
  unsigned char event; // Indexes the event set
  unsigned char hits;  // Number of times the event has been shot
  int           time;  // Point at which the event will do something,
                       // e.g. terminate

} gridElement;

typedef struct _event event;

struct _event {

  event         *next;
  event         *prev;
  float          x, y;   // Actual position of the event
  unsigned char  gridX, gridY; // Grid position of the event
  unsigned char  anim;   // E_LEFTANIM, etc, or 0

};

typedef struct {

  SDL_Surface   *pixels; // Pointer to the sprite surface
  unsigned char  x;      // Horizontal offset
  unsigned char  y;      // Vertical offset

} sprite;

typedef struct {

  sprite         sprites[19]; // Pointers to members of the sprite set
  signed char    y[19];       // Vertical offsets (for pickup wobble effect)
  unsigned char  frames;      // Number of frames

} anim;

typedef struct {

    char  anims[PANIMS];
    int   anim;
    int   facing;
    int   viewX, viewY;
    int   viewW, viewH;
    float x, y;
    float dx, dy;
    int   jumpDuration;
    int   jumpTime;
    int   score;
    int   energy;
    float energyBar;
    int   lives;
    int   reaction;
    int   reactionTime;
    int   ammo[4];
    int   ammoType; /* -1 = blaster, 0 = toaster, 1 = missiles, 2 = bouncer
                       3 = TNT */
    int   shield; /* 0 = none, 1 = 1 yellow, 2 = 2 yellow, 3 = 1 orange,
                     4 = 2 orange, 5 = 3 orange, 6 = 4 orange */
    int   floating; // 0 = normal, 1 = boarding/birding/whatever

} player;

typedef struct _bullet bullet;

struct _bullet {

  bullet *next;
  int     origin; // Negative: x coordinate of the event that fired
                  // Positive: The player who fired
  int     type;   // Negative: y coordinate of the event that fired
                  // Positive: 0 = blaster, 1 = toaster, 2 = missiles,
                  //           4 = bouncer, 5 = bird ammo
  float   x;
  float   y;
  float   dx;
  float   dy;     // Speed in pixels per second
  int     time;   // The time at which the bullet will self-destruct

};


// Variables

Extern struct {

  // It's pretty safe to assume there's one of these, but that's about it
  SDL_Surface *pixels;
  SDL_Color    palette[256];

} cutscene;


Extern struct {

  SDLKey key;
  Uint8  state;

} keys[8];


Extern char *path; // Path to game data

Extern SDL_Surface *panel;
Extern SDL_Surface *panelAmmo[5];

Extern font *font2;         /* Taken from .0FN file name */
Extern font *fontbig;       /* Taken from .0FN file name */
Extern font *fontiny;       /* Taken from .0FN file name */
Extern font *fontmn1;       /* Taken from .0FN file name */
Extern font *fontmn2;       /* Taken from .0FN file name */
Extern font *redFontmn2;    /* Redenned version of fontmn2 */
Extern font panelBigFont;   /* Not a font file, found in PANEL.000 */
Extern font panelSmallFont; /* Not a font file, found in PANEL.000 */

Extern SDL_Surface *menuScreens[14];
Extern SDL_Color    menuPalettes[3][256];
Extern int          episodes;

Extern gridElement    grid[LH][LW]; // All levels are the same size
Extern unsigned char  eventSet[EVENTS][ELENGTH]; // Not all used
Extern event         *firstEvent, *unusedEvent;
Extern sprite        *spriteSet; // 208 of which are usually in mainchar.000
Extern int            sprites;
Extern anim           animSet[ANIMS];
Extern SDL_Surface   *tileSet;
Extern char           mask[240][64]; // At most 240 tiles, all with 8 * 8 masks
Extern bullet        *firstBullet, *unusedBullet;
Extern int            time;
Extern int            skyOrb;
Extern player        *players;
Extern int            nPlayers;
Extern player        *localPlayer;
Extern char          *sceneFile;
Extern SDL_Color      levelPalette[256];
Extern SDL_Color      levelBGPalette[255];
Extern paletteEffect *bgPE;

Extern paletteEffect *firstPE;
Extern int            bgScale;

Extern SDL_Surface *screen;
Extern SDL_Color   *currentPalette;
Extern int   screenW, screenH, fullscreen;
Extern float fps;
Extern float spf;

Extern unsigned char world, level, nextworld, nextlevel;
Extern unsigned char difficulty;
Extern unsigned char checkX, checkY;


// Functions in bonus.c

Extern int  loadBonus (char * fn);
Extern void freeBonus (void);
Extern void bonusLoop (void);


// Functions in events.c

Extern void createPlayerBullet (player *source, int ticks);
Extern void createEventBullet  (event *source, int ticks);
Extern void removeBullet       (bullet * previous);
Extern void freeBullets        (void);
Extern void createEvent        (int x, int y);
Extern void removeEvent        (event * previous);
Extern void freeEvents         (void);
Extern void processEvent       (event *evt, int ticks);


// Functions in font.c

Extern font * loadFont   (char * fn, SDL_Color *palette);
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
Extern void            levelLoop     (void);


// Functions in main.c

Extern FILE          * fopenFromPath      (char * fn);
Extern unsigned char * loadRLE            (FILE * f, int size);
Extern void            skipRLE            (FILE * f);
Extern SDL_Surface   * createSurface      (unsigned char * pixels,
                                           SDL_Color *palette, int width,
                                           int height);
Extern SDL_Surface   * createBlankSurface (SDL_Color *palette);
Extern void            loadPalette        (SDL_Color *palette, FILE *f);
Extern int             loadMain           (void);
Extern void            usePalette         (SDL_Color *palette);
Extern int             loop               (void);
Extern void            quit               (void);

#define loadSurface(file, palette, width, height) \
  createSurface(loadRLE(file, (width) * (height)), palette, width, height)

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



