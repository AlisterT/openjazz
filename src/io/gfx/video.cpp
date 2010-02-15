
/*
 *
 * video.cpp
 *
 * 13th July 2009: Created graphics.cpp from parts of util.cpp
 * 26th July 2009: Renamed graphics.cpp to video.cpp
 *
 * Part of the OpenJazz project
 *
 *
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

/*
 * Contains graphics utility functions.
 *
 */


#include "video.h"

#include <string.h>


SDL_Surface * createSurface (unsigned char * pixels, int width, int height) {

	SDL_Surface *ret;
	int y;

	// Create the surface
	ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

	// Set the surface's palette
	SDL_SetPalette(ret, SDL_LOGPAL, logicalPalette, 0, 256);

	// Upload pixel data to the surface
	if (SDL_MUSTLOCK(ret)) SDL_LockSurface(ret);

	for (y = 0; y < height; y++)
		memcpy(((unsigned char *)(ret->pixels)) + (ret->pitch * y),
			pixels + (width * y), width);

	if (SDL_MUSTLOCK(ret)) SDL_UnlockSurface(ret);

	// Free redundant pixel data
	delete[] pixels;

	return ret;

}


void createFullscreen () {

	SDL_ShowCursor(SDL_DISABLE);

#ifdef WIZ
	screen = SDL_SetVideoMode(screenW, screenH, 8,
		SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE);
#else
	screen = SDL_SetVideoMode(screenW, screenH, 8,
		SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);
#endif

	SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
	SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

	/* A real 8-bit display is quite likely if the user has the right video
	card, the right video drivers, the right version of DirectX/whatever, and
	the right version of SDL. In other words, it's not likely enough. If a real
	palette is assumed when
	a) there really is a real palette, there will be an extremely small speed
		gain.
	b) the palette is emulated, there will be a HUGE speed loss.
	Therefore, assume the palette is emulated. */
	// TODO: Find a better way
	fakePalette = true;

	return;

}

#ifndef FULLSCREEN_ONLY
void createWindow () {

	screen = SDL_SetVideoMode(screenW, screenH, 8,
		SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);

	SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
	SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

	SDL_ShowCursor(SDL_ENABLE);

	/* Assume that in windowed mode the palette is being emulated.
	This is extremely likely. */
	// TODO: Find a better way
	fakePalette = true;

	return;

}
#endif


void usePalette (SDL_Color *palette) {

	// Make palette changes invisible until the next draw. Hopefully.
	clearScreen(SDL_MapRGB(screen->format, 0, 0, 0));
	SDL_Flip(screen);

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

	return;

}


void restorePalette (SDL_Surface *surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

	return;

}


void clearScreen (int index) {

	SDL_FillRect(screen, NULL, index);

	return;

}


void drawRect (int x, int y, int width, int height, int index) {

	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;

	SDL_FillRect(screen, &dst, index);

	return;

}


