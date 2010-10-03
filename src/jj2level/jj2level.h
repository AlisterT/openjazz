
/**
 *
 * @file jj2level.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 29th June 2010: Created jj2level.h from parts of level.h
 * 2nd July 2010: Created jj2event.h from parts of jj2level.h
 *
 * @section Licence
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


#ifndef _JJ2LEVEL_H
#define _JJ2LEVEL_H


#include "baselevel.h"
#include "io/gfx/anim.h"
#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Number of layers
#define LAYERS 8

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

	public:
		JJ2Layer  ();
		JJ2Layer  (int newWidth, int newHeight, int flags);
		~JJ2Layer ();

		bool getFlipped (int x, int y);
		int  getHeight  ();
		int  getTile    (int x, int y);
		int  getWidth   ();
		void setFrame   (unsigned char x, unsigned char y, unsigned char frame);
		void setTile    (unsigned char x, unsigned char y, unsigned short int tile, int tiles);

		void draw       (SDL_Surface* tileSet, SDL_Surface* flippedTileSet);

};

class JJ2Event;
class JJ2LevelPlayer;

/// JJ2 level
class JJ2Level : public BaseLevel {

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
		JJ2Layer*     layers[LAYERS]; ///< All layers
		JJ2Layer*     layer; ///< Layer 4
		JJ2Modifier** mods; ///< Modifier events for each tile in layer 4
		int           nAnimSets; ///< Number of animation sets
		bool          TSF; ///< 1.24 level
		unsigned char difficulty; ///< Difficulty setting (0 = easy, 1 = medium, 2 or 3 = hard)
		fixed         waterLevel; ///< Height of water
		fixed         waterLevelTarget; ///< Future height of water
		fixed         waterLevelSpeed; ///< Rate of water level change

		void createEvent (int x, int y, unsigned char* data);
		int  load        (char* fileName, unsigned char diff, bool checkpoint);
		void loadSprite  (unsigned char* parameters, unsigned char* compressedPixels, Sprite* sprite, Sprite* flippedSprite);
		int  loadSprites ();
		int  loadTiles   (char* fileName);

		int  step        ();
		void draw        ();

	public:
		JJ2Level  (char* fileName, unsigned char diff, bool checkpoint, bool multi);
		~JJ2Level ();

		bool         checkMaskDown (fixed x, fixed y, bool drop);
		bool         checkMaskUp   (fixed x, fixed y);
		Anim*        getAnim       (int set, int anim, bool flipped);
		JJ2Modifier* getModifier   (unsigned char gridX, unsigned char gridY);
		Sprite*      getSprite     (unsigned char sprite);
		fixed        getWaterLevel ();
		void         setFrame      (unsigned char gridX, unsigned char gridY, unsigned char frame);
		void         setNext       (char* fileName);
		void         setWaterLevel (unsigned char gridY, bool instant);
		void         warp          (JJ2LevelPlayer *player, int id);

		void         receive       (unsigned char* buffer);

		int          play          ();

};


// Variable

EXTERN JJ2Level* jj2Level; //< JJ2 level

#endif

