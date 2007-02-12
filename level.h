
/*
 *
 * level.h
 * Created on the 31st of January 2006
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2006 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#include "player.h"


// Constants

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

// Bullet types
#define B_BLASTER  41
#define B_LTOASTER 42
#define B_RTOASTER 51
#define B_LMISSILE 52
#define B_RMISSILE 43
#define B_BOUNCER  49
#define B_TNT      130

// Displayed statistics
#define S_NONE    0
#define S_PLAYERS 1
#define S_SCREEN  2

// General
#define LW        256 /* Level width */
#define LH         64 /* Level height */
#define BLACK      31
#define EVENTS    127
#define ELENGTH    32 /* Length of events, in bytes */
#define ANIMS     128
#define TW         32 /* Tile width */
#define TH         32 /* Tile height */
#define SKEY      254 /* As in sprite colour key */
#define TKEY      127 /* As in tileset colour key */
#define HURTTIME 1000
#define EOLTIME  4000
#define FLASHTIME 100



// Datatypes


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
  fixed          x, y;   // Actual position of the event
  unsigned char  gridX, gridY; // Grid position of the event
  unsigned char  anim;   // E_LEFTANIM, etc, or 0
  int            flashTime;

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

typedef struct _bullet bullet;

struct _bullet {

  bullet *next;
  int     origin; // Negative: x coordinate of the event that fired
                  // Positive: The player who fired
  int     type;   // Negative: y coordinate of the event that fired
                  // Positive: 0 = blaster, 1 = toaster, 2 = missiles,
                  //           4 = bouncer, 5 = bird ammo
  fixed   x;
  fixed   y;
  fixed   dx;
  fixed   dy;     // Speed in pixels per second
  int     time;   // The time at which the bullet will self-destruct

};


// Variables

Lextern gridElement    grid[LH][LW]; // All levels are the same size
Lextern signed char    eventSet[EVENTS][ELENGTH]; // Not all used
Lextern event         *firstEvent, *unusedEvent;
Lextern sprite        *spriteSet; // 208 of which are usually in mainchar.000
Lextern int            sprites;
Lextern anim           animSet[ANIMS];
Lextern SDL_Surface   *tileSet;
Lextern char           mask[240][64]; // At most 240 tiles, all with 8 * 8 masks
Lextern bullet        *firstBullet, *unusedBullet;
Lextern int            endTicks;
Lextern int            skyOrb;
Lextern char          *sceneFile;
Lextern SDL_Color      levelPalette[256];
Lextern paletteEffect *bgPE;
Lextern unsigned char  checkX, checkY;
Lextern int            stats;


// Functions in events.c

Lextern void createPlayerBullet (player *source, int ticks);
Lextern void createEventBullet  (event *source, int ticks);
Lextern void removeBullet       (bullet * previous);
Lextern void freeBullets        (void);
Lextern void createEvent        (int x, int y);
Lextern void removeEvent        (event * previous);
Lextern void freeEvents         (void);
Lextern void playEventFrame     (event *evt, int ticks);
Lextern void drawEvent          (event *evt, int ticks);


// Functions in level.c

Lextern int  loadSprites   (char * fn);
Lextern void freeSprites   (void);
Lextern int  loadTiles     (char * fn);
Lextern int  loadLevel     (char * fn);
Lextern void freeLevel     (void);
Lextern int  loadMacro     (char * fn);
Lextern void freeMacro     (void);
Lextern int  checkMask     (fixed x, fixed y);
Lextern int  checkMaskDown (fixed x, fixed y);
Lextern int  runLevel      (char * fn);


