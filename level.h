
/*
 *
 * level.h
 *
 * Created on the 31st of January 2006 from parts of OpenJazz.h
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _LEVEL_H
#define _LEVEL_H


#include "bullet.h"
#include "events.h"
#include "scene.h"
#include "sprite.h"


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
#define TW         32 /* Tile width */
#define TH         32 /* Tile height */
#define TKEY      127 /* Tileset colour key */

// Fade delays
#define T_START 500
#define T_END   1000


// Datatype

typedef struct {

	unsigned char tile;  // Indexes the tile set
	unsigned char bg;    /* 0 = Normal background
	                        1 = Black background */
	unsigned char event; // Indexes the event set
	unsigned char hits;  // Number of times the event has been shot
	int           time;  /* Point at which the event will do something, e.g.
	                        e.g. terminate */

} GridElement;


// Class

class Level {

	protected:
		char          *sceneFile;
		Sprite        *spriteSet; // 208 of which are usually in mainchar.000
		SDL_Surface   *tileSet;
		Anim           animSet[ANIMS];
		char           miscAnims[4];
		signed char    bulletSet[BULLETS][BLENGTH];
		signed char    eventSet[EVENTS][ELENGTH]; // Not all used
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
		int            pathLength;
		int            ticks, endTime;
		int            timeBonus;
		int            enemies, items;
		fixed          waterLevel;
		int            gameMode;
		fixed          energyBar;

		int  loadSprites (char *fileName);
		int  loadTiles   (char *fileName);

	protected:
		int  load      (char *fileName, unsigned char diff, bool checkpoint);
		int  playFrame ();
		void draw      ();

	public:
		Event  *firstEvent;
		Bullet *firstBullet;
		int    *pathX;
		int    *pathY;
		int     pathNode;

		Level                       ();
		Level                       (char *fileName, unsigned char diff,
			bool checkpoint);
		virtual ~Level              ();
		bool          checkMask     (fixed x, fixed y);
		bool          checkMaskDown (fixed x, fixed y);
		bool          checkSpikes   (fixed x, fixed y);
		void          setNext       (int nextLevel, int nextWorld);
		void          setTile       (unsigned char gridX, unsigned char gridY,
			unsigned char tile);
		signed char * getEvent      (unsigned char gridX, unsigned char gridY);
		unsigned char getEventHits  (unsigned char gridX, unsigned char gridY);
		int           getEventTime  (unsigned char gridX, unsigned char gridY);
		void          clearEvent    (unsigned char gridX, unsigned char gridY);
		int           hitEvent      (unsigned char gridX, unsigned char gridY);
		void          setEventTime  (unsigned char gridX, unsigned char gridY,
			int time);
		signed char * getBullet     (unsigned char bullet);
		Sprite *      getSprite     (unsigned char sprite);
		Anim *        getAnim       (unsigned char anim);
		Anim *        getMiscAnim   (unsigned char anim);
		void          addTimer      ();
		void          setWaterLevel (unsigned char gridY);
		fixed         getWaterLevel (int phase);
		void          playSound     (int sound);
		void          win           ();
		Scene *       createScene   ();
		void          receive       (unsigned char *buffer);
		virtual int   run           ();

};


class DemoLevel : public Level {

	private:
		unsigned char *macro;

	public:
		DemoLevel  (char *fileName);
		~DemoLevel ();
		int run    ();

};


// Variables

Extern Level         *level;
Extern unsigned char  checkX, checkY;
Extern fixed          viewX, viewY;

#endif

