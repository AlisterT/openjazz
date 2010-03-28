
/*
 *
 * level.h
 *
 * 31st January 2006: Created level.h from parts of OpenJazz.h
 * 4th February 2009: Created events.h from parts of level.h
 * 19th March 2009: Created sprite.h from parts of level.h
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _LEVEL_H
#define _LEVEL_H


#include "io/gfx/anim.h"
#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Displayed statistics
#define S_NONE    0
#define S_PLAYERS 1
#define S_SCREEN  2

// General
#define LW        256 /* Level width */
#define LH         64 /* Level height */
#define EVENTS    127
#define ELENGTH    32 /* Length of events, in bytes */
#define BULLETS    32
#define BLENGTH    20 /* Length of bullets, in bytes */
#define ANIMS     128
#define PATHS      16
#define TKEY      127 /* Tileset colour key */

// Stages
#define LS_NORMAL      0
#define LS_SUDDENDEATH 1
#define LS_END         2

// Fade delays
#define T_START 500
#define T_END   1000


// Macros

// For converting between tile positions and int/fixed values
#define FTOT(x) ((x) >> 15)
#define TTOF(x) ((x) << 15)
#define ITOT(x) ((x) >> 5)
#define TTOI(x) ((x) << 5)


// Datatypes

typedef struct {

	unsigned char tile;  // Indexes the tile set
	unsigned char bg;    // 0 = Effect background, 1 = Black background
	unsigned char event; // Indexes the event set
	unsigned char hits;  // Number of times the event has been shot
	int           time;  /* Point at which the event will do something, e.g.
		terminate */

} GridElement;

typedef struct {

	short int     *x;
	short int     *y;
	unsigned char  length;
	unsigned char  node;

} EventPath;


// Classes

class Bullet;
class Event;
class Player;
class Scene;

class Level {

	private:
		char          *sceneFile;
		Sprite        *spriteSet; // 208 of which are usually in mainchar.000
		SDL_Surface   *tileSet;
		Anim           animSet[ANIMS];
		char           miscAnims[4];
		signed char    bulletSet[BULLETS][BLENGTH];
		signed char    eventSet[EVENTS][ELENGTH];
		char           mask[240][64]; // At most 240 tiles, all with 8 * 8 masks
		GridElement    grid[LH][LW]; // All levels are the same size
		int            soundMap[32];
		SDL_Color      palette[256];
		SDL_Color      skyPalette[256];
		bool           sky;
		unsigned char  skyOrb;
		int            sprites;
		int            levelNum, worldNum, nextLevelNum, nextWorldNum;
		unsigned char  difficulty;
		unsigned int   endTime;
		int            enemies, items;
		fixed          waterLevel;
		fixed          waterLevelTarget;
		fixed          waterLevelSpeed;
		fixed          energyBar;
		int            stage;
		float          smoothfps;

		int loadSprites (char *fileName);
		int loadTiles   (char *fileName);

	protected:
		unsigned int tickOffset, prevStepTicks, prevTicks, ticks;

		int  load      (char *fileName, unsigned char diff, bool checkpoint);
		int  step      ();
		void draw      (int stats);
		void timeCalcs (bool paused);

	public:
		Event     *firstEvent;
		Bullet    *firstBullet;
		EventPath  path[PATHS];

		Level                       ();
		Level                       (char *fileName, unsigned char diff,
			bool checkpoint);
		virtual ~Level              ();
		bool          checkMask     (fixed x, fixed y);
		bool          checkMaskDown (fixed x, fixed y);
		bool          checkSpikes   (fixed x, fixed y);
		void          setNext       (int nextLevel, int nextWorld);
		void          setTile       (unsigned char gridX, unsigned char gridY, unsigned char tile);
		signed char * getEvent      (unsigned char gridX, unsigned char gridY);
		unsigned char getEventHits  (unsigned char gridX, unsigned char gridY);
		unsigned int  getEventTime  (unsigned char gridX, unsigned char gridY);
		void          clearEvent    (unsigned char gridX, unsigned char gridY);
		int           hitEvent      (unsigned char gridX, unsigned char gridY, Player *source);
		void          setEventTime  (unsigned char gridX, unsigned char gridY, unsigned int time);
		signed char * getBullet     (unsigned char bullet);
		Sprite *      getSprite     (unsigned char sprite);
		Anim *        getAnim       (unsigned char anim);
		Anim *        getMiscAnim   (unsigned char anim);
		void          addTimer      ();
		void          setWaterLevel (unsigned char gridY);
		fixed         getWaterLevel ();
		void          playSound     (int sound);
		void          setStage      (int stage);
		int           getStage      ();
		Scene *       createScene   ();
		void          receive       (unsigned char *buffer);
		virtual int   play          ();

};


class DemoLevel : public Level {

	private:
		unsigned char *macro;

	public:
		DemoLevel  (const char *fileName);
		~DemoLevel ();
		int play   ();

};


// Variables

EXTERN Level         *level;
EXTERN fixed          viewX, viewY;

#endif

