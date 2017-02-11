
/**
 *
 * @file jj1bonuslevel.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 3rd February 2009: Created bonus.h
 * 1st August 2012: Renamed bonus.h to jj1bonuslevel.h
 *
 * @section Licence
 * Copyright (c) 2009-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _BONUS_H
#define _BONUS_H

#include "level/level.h"

#include "io/gfx/anim.h"

#include <SDL/SDL.h>


// Constants

// General
#define BLW    256 /* Bonus level width */
#define BLH    256 /* Bonus level height */
#define BANIMS  32

#define T_BONUS_END 2000


// Datatype

/// JJ1 bonus level grid element
typedef struct {

	unsigned char tile;  ///< Indexes the tile set
	unsigned char event; ///< Event type

} JJ1BonusLevelGridElement;


// Classes

class Font;

/// JJ1 bonus level
class JJ1BonusLevel : public Level {

	private:
		SDL_Surface*             tileSet; ///< Tile images
		SDL_Surface*             background; ///< Background image
		Font*                    font; ///< On-screen message font
		Sprite*                  spriteSet; ///< Sprite images
		Anim                     animSet[BANIMS]; ///< Animations
		JJ1BonusLevelGridElement grid[BLH][BLW]; ///< Level grid
		char                     mask[60][64]; ///< Tile masks (at most 60 tiles, all with 8 * 8 masks)
		fixed                    direction; ///< Player's direction

		int  loadSprites ();
		int  loadTiles   (char* fileName);
		bool isEvent     (fixed x, fixed y);
		int  step        ();
		void draw        ();

	public:
		JJ1BonusLevel  (Game* owner, char* fileName, bool multi);
		~JJ1BonusLevel ();

		bool checkMask (fixed x, fixed y);
		void receive   (unsigned char* buffer);
		int  play      ();

};

#endif

