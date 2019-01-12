
/**
 *
 * @file jj1level.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 4th February 2009: Created events.h from parts of level.h
 * - 19th March 2009: Created sprite.h from parts of level.h
 * - 30th March 2010: Created baselevel.h from parts of level.h
 * - 29th June 2010: Created jj2level.h from parts of level.h
 * - 1st August 2012: Renamed level.h to jj1level.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _LEVEL_H
#define _LEVEL_H


#include "level/level.h"
#include "io/gfx/anim.h"
#include "OpenJazz.h"


// Constants

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
#define PA_LBORED   20
#define PA_RBORED   21
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
#define PA_LSUCK    32
#define PA_RSUCK    33
#define PA_LSTOP    34 /* SUCK and STOP refer almost always to the same animation */
#define PA_RSTOP    35
#define PA_RSPRING  36
#define PA_LSPRING  37 /* Surely these are the wrong way round? */

#define JJ1PANIMS   38 /* Number of player animations. May be higher. */

// Miscellaneous animations
#define MA_SPARKLE    0
#define MA_DEVHEAD    1
#define MA_EXPLOSION1 2
#define MA_EXPLOSION2 3
#define MA_4SHIELD    4
#define MA_LBOARD     5
#define MA_RBOARD     6
#define MA_LBIRD      7
#define MA_RBIRD      8
#define MA_ICY        9
#define MA_1SHIELD    10
#define JJ1MANIMS     11

// Black palette index
#define LEVEL_BLACK 31

// Fade delays
#define T_START 500
#define T_END   1000


// Datatypes

/// JJ1 level grid element
typedef struct {

	unsigned char tile; ///< Indexes the tile set
	unsigned char bg; ///< 0 = Effect background, 1 = Black background
	unsigned char event; ///< Indexes the event set
	unsigned char hits; ///< Number of times the event has been shot
	int           time; ///< Point at which the event will do something, e.g. terminate

} GridElement;

/// JJ1 level event type
typedef struct {

	unsigned char anims[6]; ///< Indices of animations
	signed char   difficulty; ///< The minimum difficulty level at which the event is used
	signed char   reflection; ///< Whether or not to show a reflection
	signed char   movement; ///< Movement type
	signed char   magnitude; ///< Usage depends on event type
	signed char   strength; ///< Number of hits required to destroy the event
	signed char   modifier; ///< Modifier
	unsigned char points; ///< Points obtained by getting/destroying the event
	unsigned char bullet; ///< Type of bullet the event fires
	unsigned char bulletPeriod; ///< The time between successive bullet shots
	unsigned char speed; ///< The speed at which the event moves
	unsigned char animSpeed; ///< The speed of the event's animation
	unsigned char sound; ///< The sound played on the appropriate trigger
	signed char   multiA; ///< Usage depends on event type
	signed char   multiB; ///< Usage depends on event type
	signed char   pieceSize; ///< Size of pieces in bridges, swinging balls chains, etc.
	signed char   pieces; ///< Number of pieces in bridges, swinging ball chains, etc.
	signed char   angle; ///< Initial angle of swinging balls, etc.

} JJ1EventType;

/// Pre-defined JJ1 event movement path
typedef struct {

	short int*    x; ///< X-coordinates for each node
	short int*    y; ///< Y-coordinates for each node
	unsigned char length; ///< Number of nodes

} JJ1EventPath;


// Classes

class Font;
class JJ1Bullet;
class JJ1Event;
class JJ1LevelPlayer;

/// JJ1 level
class JJ1Level : public Level {

	private:
		SDL_Surface*  tileSet; ///< Tile images
		SDL_Surface*  panel; ///< HUD background image
		SDL_Surface*  panelAmmo[6]; ///< HUD ammo type images
		JJ1Event*     events; ///< Active events
		JJ1Bullet*    bullets; ///< Active bullets
		char*         sceneFile; ///< File name of cutscene to play when level has been completed
		Sprite*       spriteSet; ///< Sprites
		Anim          animSet[ANIMS]; ///< Animations
		char          miscAnims[JJ1MANIMS]; ///< Further animations
		char          playerAnims[JJ1PANIMS]; ///< Default player animations
		signed char   bulletSet[BULLETS][BLENGTH]; ///< Bullet types
		JJ1EventType  eventSet[EVENTS]; ///< Event types
		char          mask[240][64]; ///< Tile masks. At most 240 tiles, all with 8 * 8 masks
		GridElement   grid[LH][LW]; ///< Level grid. All levels are the same size
		SDL_Color     skyPalette[256]; ///< Full palette for sky background
		bool          sky; ///< Whether or not to use sky background
		unsigned char skyOrb; ///< The tile to use as the background sun/moon/etc.
		int           levelNum; ///< Number of current level
		int           worldNum; ///< Number of current world
		int           nextLevelNum; ///< Number of next level
		int           nextWorldNum; ///< Number of next world
		int           enemies; ///< Number of enemies to kill
		fixed         waterLevel; ///< Height of water
		fixed         waterLevelTarget; ///< Future height of water
		fixed         waterLevelSpeed; ///< Rate of water level change
		fixed         energyBar; ///< HUD energy bar fullness
		int           ammoType; ///< HUD ammo type
		fixed         ammoOffset; ///< HUD ammo offset

		void deletePanel  ();
		int  loadPanel    ();
		void loadSprite   (File* file, Sprite* sprite);
		int  loadSprites  (char* fileName);
		int  loadTiles    (char* fileName);
		int  playBonus    ();

	protected:
		Font* font; ///< On-screen message font
		char* musicFile; ///< Music file name

		JJ1Level (Game* owner);

		int  load (char* fileName, bool checkpoint);
		int  step ();
		void draw ();

	public:
		JJ1EventPath path[PATHS]; ///< Pre-defined event movement paths

		JJ1Level          (Game* owner, char* fileName, bool checkpoint, bool multi);
		virtual ~JJ1Level ();

		bool          checkMaskUp   (fixed x, fixed y);
		bool          checkMaskDown (fixed x, fixed y);
		bool          checkSpikes   (fixed x, fixed y);
		int           getWorld      ();
		void          setNext       (int nextLevel, int nextWorld);
		void          setTile       (unsigned char gridX, unsigned char gridY, unsigned char tile);
		JJ1Event*     getEvents     ();
		JJ1EventType* getEvent      (unsigned char gridX, unsigned char gridY);
		unsigned char getEventHits  (unsigned char gridX, unsigned char gridY);
		unsigned int  getEventTime  (unsigned char gridX, unsigned char gridY);
		void          clearEvent    (unsigned char gridX, unsigned char gridY);
		int           hitEvent      (unsigned char gridX, unsigned char gridY, int hits, JJ1LevelPlayer* source, unsigned int time);
		void          setEventTime  (unsigned char gridX, unsigned char gridY, unsigned int time);
		Sprite*       getSprite     (unsigned char sprite);
		Anim*         getAnim       (unsigned char anim);
		Anim*         getMiscAnim   (unsigned char anim);
		Anim*         getPlayerAnim (unsigned char anim);
		void          createBullet  (JJ1LevelPlayer* sourcePlayer, unsigned char gridX, unsigned char gridY, fixed startX, fixed startY, unsigned char bullet, bool facing, unsigned int time);
		void          setWaterLevel (unsigned char gridY);
		fixed         getWaterLevel ();
		void          flash         (unsigned char red, unsigned char green, unsigned char blue, int duration);
		void          receive       (unsigned char* buffer);
		virtual int   play          ();

};

/// JJ1 level played as a demo
class JJ1DemoLevel : public JJ1Level {

	private:
		unsigned char* macro; ///< Sequence of player control codes

	public:
		JJ1DemoLevel  (Game* owner, const char* fileName);
		~JJ1DemoLevel ();

		int play   ();

};


// Variables

EXTERN JJ1Level* level; ///< JJ1 level

#endif

