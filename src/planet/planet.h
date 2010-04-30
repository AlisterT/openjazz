
/*
 *
 * planet.h
 *
 * 3rd of February 2009: Created planet.h
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2009 Alister Thomson
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

class Planet {

	private:
		SDL_Color palette[256];
		Sprite    sprite;
		char*     name;
		int       id;

	public:
		Planet  (char * fileName, int previous);
		~Planet ();

		int getId ();
		int play  ();

};

#endif

