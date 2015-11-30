
/**
 *
 * @file jj1planet.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 3rd February 2009: Created planet.h
 * 1st August 2012: Renamed planet.h to jj1planet.h
 *
 * @section Licence
 * Copyright (c) 2009-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _PLANET_H
#define _PLANET_H


#include "io/gfx/sprite.h"

#include <SDL/SDL.h>


// Class

/// Planet approach sequence
class JJ1Planet {

	private:
		SDL_Color palette[256]; /// Palette
		Sprite    sprite; /// Planet image
		char*     name; /// Planet name
		int       id; /// World number

	public:
		JJ1Planet  (char * fileName, int previous);
		~JJ1Planet ();

		int getId ();
		int play  ();

};

#endif

