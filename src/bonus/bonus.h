
/*
 *
 * bonus.h
 *
 * 3rd February 2009: Created bonus.h
 *
 * Part of the OpenJazz project
 *
 *
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

#include "baselevel.h"

#include "io/gfx/anim.h"

#include <SDL/SDL.h>


// Constants

// General
#define BLW    256 /* Bonus level width */
#define BLH    256 /* Bonus level height */
#define BANIMS  32

#define T_BONUS_END 2000


// Datatype

typedef struct {

	unsigned char tile;  // Indexes the tile set
	unsigned char event;

} BonusGridElement;


// Classes

class Font;

class Bonus : public BaseLevel {

	private:
		SDL_Surface*     background;
		Font*            font;
		Anim             animSet[BANIMS];
		BonusGridElement grid[BLH][BLW];
		char             mask[60][64]; // At most 60 tiles, all with 8 * 8 masks
		fixed            direction;

		int  loadSprites ();
		int  loadTiles   (char* fileName);
		bool isEvent     (fixed x, fixed y);
		int  step        ();
		void draw        ();

	public:
		Bonus  (char* fileName, unsigned char diff);
		~Bonus ();

		bool checkMask (fixed x, fixed y);
		void receive   (unsigned char* buffer);
		int  play      ();

};

#endif

