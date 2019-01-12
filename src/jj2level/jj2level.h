
/**
 *
 * @file jj2level.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 29th June 2010: Created jj2level.h from parts of level.h
 * - 2nd July 2010: Created jj2event.h from parts of jj2level.h
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


#ifndef _JJ2LEVEL_H
#define _JJ2LEVEL_H


#include "level/level.h"
#include "io/gfx/anim.h"
#include "OpenJazz.h"


// Constants

// Number of layers
#define LAYERS 8

// Player animations
#define JJ2PA_BOARD        0
#define JJ2PA_BOARDSW      1
#define JJ2PA_STOMP        2
#define JJ2PA_DEAD         3
#define JJ2PA_DIE          4
#define JJ2PA_CROUCH1      5
#define JJ2PA_CROUCHED     6
#define JJ2PA_CROUCHSHOOT  7
#define JJ2PA_CROUCH2      8
#define JJ2PA_VINE         9
#define JJ2PA_EXIT1        10
#define JJ2PA_FALL         11
#define JJ2PA_STOMPING     12
#define JJ2PA_LAND         13
#define JJ2PA_STANDSHOOT   14
#define JJ2PA_STANDSHOOTUP 15
#define JJ2PA_WHIP1        16
#define JJ2PA_UNFROG       17
#define JJ2PA_HOOKWHIP     18
#define JJ2PA_HOOKDIAG     19
#define JJ2PA_HOOKSHOOTUP  20
#define JJ2PA_HOOK1        21
#define JJ2PA_HOOK2        22
#define JJ2PA_HOOKWHIPUP   23
#define JJ2PA_HOOKSHOOT    24
#define JJ2PA_HELI         25
#define JJ2PA_HELIWHIP     26
#define JJ2PA_HELISHOOT    27
#define JJ2PA_HPOLE        28
#define JJ2PA_HURT         29
#define JJ2PA_WAIT1        30
#define JJ2PA_WAIT2        31
#define JJ2PA_WAIT3        32
#define JJ2PA_WAIT4        33
#define JJ2PA_WAIT5        34
#define JJ2PA_FALLWHIP     35
#define JJ2PA_FALLSHOOT    36
#define JJ2PA_FLOAT1       37
#define JJ2PA_FLOAT2       38
#define JJ2PA_UP1          39
#define JJ2PA_EDGE         40
#define JJ2PA_CARRY        41
#define JJ2PA_UNLOAD       42
#define JJ2PA_LOAD         43
#define JJ2PA_LOOKUP       44
#define JJ2PA_WOOZYWALK    45
#define JJ2PA_PUSH         46
#define JJ2PA_WHIP2        47
#define JJ2PA_EXIT2        48
#define JJ2PA_SPEED1       49
#define JJ2PA_SPEED2       50
#define JJ2PA_FALLMOVE     51
#define JJ2PA_JUMP         52
#define JJ2PA_BALL         53
#define JJ2PA_WALKSHOOT    54
#define JJ2PA_RUN          55
#define JJ2PA_SPEEDRUN     56
#define JJ2PA_STOP1        57
#define JJ2PA_MYSTERY      58
#define JJ2PA_STOP2        59
#define JJ2PA_UP2          60
#define JJ2PA_STAND        61
#define JJ2PA_POWER        62
#define JJ2PA_POWEREND     63
#define JJ2PA_POWERSTART   64
#define JJ2PA_WOOZYSTAND   65
#define JJ2PA_SWIMDOWN     66
#define JJ2PA_SWIM         67
#define JJ2PA_SWIMDIAGDOWN 68
#define JJ2PA_SWIMDIAGUP   69
#define JJ2PA_SWIMUP       70
#define JJ2PA_VINESDIAG    71
#define JJ2PA_WARPOUT      72
#define JJ2PA_WARPFALLIN   73
#define JJ2PA_WARPFALL     74
#define JJ2PA_WARPFALLOUT  75
#define JJ2PA_WARPIN       76
#define JJ2PA_VPOLE        77

#define JJ2PANIMS          78 /* Number of player animations. */

// Black palette index
#define JJ2_BLACK 0


// Datatypes

/// JJ2 level tile
typedef struct {

	unsigned short int tile;    ///< Indexes the tile set
	unsigned char      frame;   ///< Current frame being used (for animated tiles)
	bool               flipped; ///< Whether or not the tile image and mask are flipped

} JJ2Tile;

/// JJ2 level tile modifier event
typedef struct {

	unsigned char type;
	int properties;

} JJ2Modifier;


// Classes

class Font;

///< JJ2 level parallaxing layer
class JJ2Layer {

	private:
		JJ2Tile** grid; ///< Layer tiles
		int       width; ///< Width (in tiles)
		int       height; ///< Height (in tiles)
		bool      tileX; ///< Repeat horizontally
		bool      tileY; ///< Repeat vertically
		bool      limit; ///< Do not view beyond edges
		bool      warp; ///< Warp effect
		fixed     xSpeed; ///< Relative horizontal speed
		fixed     ySpeed; ///< Relative vertical speed

	public:
		JJ2Layer  ();
		JJ2Layer  (int flags, int newWidth, int newHeight, fixed newXSpeed, fixed newYSpeed);
		~JJ2Layer ();

		bool getFlipped (int x, int y);
		int  getHeight  ();
		int  getTile    (int x, int y);
		int  getWidth   ();
		void setFrame   (int x, int y, unsigned char frame);
		void setTile    (int x, int y, unsigned short int tile, bool TSF, int tiles);

		void draw       (SDL_Surface* tileSet, SDL_Surface* flippedTileSet);

};

class JJ2Event;
class JJ2LevelPlayer;

/// JJ2 level
class JJ2Level : public Level {

	private:
		SDL_Surface*  tileSet; ///< Tile images
		SDL_Surface*  flippedTileSet; ///< Tile images (flipped)
		JJ2Event*     events; ///< "Movable" events
		Font*         font; ///< On-screen message font
		char*         mask; ///< Tile masks
		char*         flippedMask; ///< Tile masks (flipped)
		char*         musicFile; ///< Music file name
		char*         nextLevel; ///< Next level file name
		Sprite*       spriteSet; ///< Sprite images
		Sprite*       flippedSpriteSet; ///< Sprite images (flipped)
		Anim**        animSets; ///< Animation sets
		Anim**        flippedAnimSets; ///< Animation sets (flipped)
		char          playerAnims[JJ2PANIMS]; ///< Player animations
		JJ2Layer*     layers[LAYERS]; ///< All layers
		JJ2Layer*     layer; ///< Layer 4
		JJ2Modifier** mods; ///< Modifier events for each tile in layer 4
		int           nAnimSets; ///< Number of animation sets
		bool          TSF; ///< 1.24 level
		fixed         waterLevel; ///< Height of water
		fixed         waterLevelTarget; ///< Future height of water
		fixed         waterLevelSpeed; ///< Rate of water level change

		void createEvent (int x, int y, unsigned char* data);
		int  load        (char* fileName, bool checkpoint);
		void loadSprite  (unsigned char* parameters, unsigned char* compressedPixels, Sprite* sprite, Sprite* flippedSprite);
		int  loadSprites ();
		int  loadTiles   (char* fileName);

		int  step        ();
		void draw        ();

	public:
		JJ2Level  (Game* owner, char* fileName, bool checkpoint, bool multi);
		~JJ2Level ();

		bool         checkMaskDown (fixed x, fixed y, bool drop);
		bool         checkMaskUp   (fixed x, fixed y);
		Anim*        getAnim       (int set, int anim, bool flipped);
		Anim*        getPlayerAnim (int character, int anim, bool flipped);
		JJ2Modifier* getModifier   (int gridX, int gridY);
		Sprite*      getSprite     (unsigned char sprite);
		fixed        getWaterLevel ();
		void         setFrame      (int gridX, int gridY, unsigned char frame);
		void         setNext       (char* fileName);
		void         setWaterLevel (int gridY, bool instant);
		void         warp          (JJ2LevelPlayer *player, int id);

		void         receive       (unsigned char* buffer);

		int          play          ();

};


// Variable

EXTERN JJ2Level* jj2Level; //< JJ2 level

#endif

