
/**
 *
 * @file jj1planet.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 3rd February 2009: Created planet.h
 * - 1st August 2012: Renamed planet.h to jj1planet.h
 *
 * @par Licence:
 * Copyright (c) 2009-2017 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _PLANET_H
#define _PLANET_H


#include "io/gfx/sprite.h"

#ifdef OJ_SDL3
	#include <SDL3/SDL.h>
#else
	#include <SDL.h>
#endif

// Class

/// Planet approach sequence
class JJ1Planet {

	private:
		SDL_Color palette[MAX_PALETTE_COLORS]; /// Palette
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

