
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

// Delays
#define T_HURT 1000
#define T_WON  6000



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

	protected:
		PaletteEffect *bgPE;
		char          *sceneFile;
		Sprite        *spriteSet; // 208 of which are usually in mainchar.000
		SDL_Surface   *tileSet;
		Anim           animSet[ANIMS];
		signed char    bulletSet[BULLETS][BLENGTH];
		signed char    eventSet[EVENTS][ELENGTH]; // Not all used
		char           mask[240][64]; // At most 240 tiles, all with 8 * 8 masks
		GridElement    grid[LH][LW]; // All levels are the same size
		SDL_Color      palette[256];
		SDL_Color      skyPalette[256];
		bool           sky;
		unsigned char  skyOrb;
		int            sprites;
		int            levelNum, worldNum, nextLevelNum, nextWorldNum;
		unsigned char  difficulty;
		int            pathLength;
		int            ticks, endTime, winTime;
		int            enemies, items;
		fixed          waterLevel;
		int            gameMode;

		int  loadSprites   (char *fn);
		int  loadTiles     (char *fn);

	protected:
		void load      (char *fn, unsigned char diff, bool checkpoint);
		int  playFrame ();
		void draw      ();

	public:
		Event       *firstEvent;
		Bullet      *firstBullet;
		signed char *pathX;
		signed char *pathY;
		int          pathNode;

		Level                       ();
		Level                       (char *fn, unsigned char diff,
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
		bool          hitEvent      (unsigned char gridX, unsigned char gridY,
			bool TNT);
		void          setEventTime  (unsigned char gridX, unsigned char gridY,
			int time);
		signed char * getBullet     (unsigned char bullet);
		Sprite *      getSprite     (unsigned char sprite);
		Anim *        getAnim       (unsigned char anim);
		Sprite *      getFrame      (unsigned char anim, unsigned char frame);
		void          addTimer      ();
		void          setWaterLevel (unsigned char gridY);
		fixed         getWaterLevel (int phase);
		void          win           ();
		Scene *       createScene   ();
		void          receive       (unsigned char *buffer);
		virtual int   run           ();

};


class DemoLevel : public Level {

	private:
		unsigned char *macro;

	public:
		DemoLevel  (char *fn);
		~DemoLevel ();
		int run    ();

};


