
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

#include <SDL/SDL.h>


// Constants

// General
#define BLW 256 /* Bonus level width */
#define BLH 256 /* Bonus level height */

#define T_BONUS_END 1500


// Class

class Bonus : public BaseLevel {

	private:
		SDL_Surface   *background;
		unsigned char  tiles[BLH][BLW];
		unsigned char  events[BLH][BLW];

		int  loadTiles (char *fileName);
		int  step ();
		void draw ();

	public:
		Bonus    (char * fileName, unsigned char diff);
		~Bonus   ();

		int play ();

};

#endif

