
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


// Constants

// Displayed statistics
#define S_NONE    0
#define S_PLAYERS 1
#define S_SCREEN  2

// General
#define LW        256 /* Level width */
#define LH         64 /* Level height */
#define BLACK      31 /* Black palette index */
#define EVENTS    127
#define ELENGTH    32 /* Length of events, in bytes */
#define BULLETS    32
#define BLENGTH    20 /* Length of bullets, in bytes */
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
	int           time;  /* Point at which the event will do something, e.g.
	                        e.g. terminate */

} GridElement;

typedef struct {

	SDL_Surface   *pixels; // Pointer to the sprite surface
	unsigned char  x;      // Horizontal offset
	unsigned char  y;      // Vertical offset

} Sprite;

typedef struct {

	Sprite         sprites[19]; // Pointers to members of the sprite set
	signed char    y[19];       // Vertical offsets (for pickup wobble effect)
	unsigned char  frames;      // Number of frames

} Anim;



// Class

class Level {

	private:
		PaletteEffect *bgPE;
		char          *sceneFile;
		Player        *localPlayer;
		Sprite        *spriteSet; // 208 of which are usually in mainchar.000
		SDL_Surface   *tileSet;
		Anim           animSet[ANIMS];
		signed char    bulletSet[BULLETS][BLENGTH];
		signed char    eventSet[EVENTS][ELENGTH]; // Not all used
		char           mask[240][64]; // At most 240 tiles, all with 8 * 8 masks
		GridElement    grid[LH][LW]; // All levels are the same size
		SDL_Color      levelPalette[256];
		int            sprites;
		unsigned char  skyOrb;
		int            levelNum, worldNum;
		unsigned char  difficulty;
		int            pathLength;
		int            endTime, winTime;
		int            enemies, items;
		fixed          waterLevel;

		int  loadSprites (char * fn);
		int  loadTiles   (char * fn);
		void createPEs   (int bgType);
		int  playFrame   (int ticks);
		void draw        (int ticks);

	public:
		char        *currentFile;
		Event       *firstEvent;
		Bullet      *firstBullet;
		signed char *pathX;
		signed char *pathY;
		SDL_Color    skyPalette[256];
		int          pathNode;

		Level                       (char *fn, unsigned char diff,
			bool checkpoint);
		~Level                      ();
		int           run           ();
		int           checkMask     (fixed x, fixed y);
		int           checkMaskDown (fixed x, fixed y);
		void          setNext       (int newLevelNum, int newWorldNum);
		GridElement * getGrid       (unsigned char gridX, unsigned char gridY);
		signed char * getEvent      (unsigned char gridX, unsigned char gridY);
		signed char * getBullet     (unsigned char bullet);
		Sprite *      getSprite     (unsigned char sprite);
		Anim *        getAnim       (unsigned char anim);
		Sprite *      getFrame      (unsigned char anim, unsigned char frame);
		void          addTimer      ();
		void          setWaterLevel (unsigned char gridY);
		fixed         getWaterLevel (int ticks);
		void          win           (int ticks);
		Scene *       createScene   ();

};


