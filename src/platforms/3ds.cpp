
/**
 *
 * @file 3ds.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "3ds.h"

#ifdef _3DS

#include <3ds.h>
#include <SDL.h>

void N3DS_SetKeyMap() {

	// reset mapping
	for (int i = 0; i < 32; i++)
		SDL_N3DSKeyBind(1 << i, SDLK_UNKNOWN);

	// only add direction keys for d-pad
	SDL_N3DSKeyBind(KEY_DUP, SDLK_UP);
	SDL_N3DSKeyBind(KEY_DDOWN, SDLK_DOWN);
	SDL_N3DSKeyBind(KEY_DLEFT, SDLK_LEFT);
	SDL_N3DSKeyBind(KEY_DRIGHT, SDLK_RIGHT);

}

#endif
