
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


#include "paletteeffects.h"
#include "video.h"

#ifdef SCALE
	#include "io/gfx/scale2x/scalebit.h"
#endif

#include <string.h>


SDL_Surface* createSurface (unsigned char * pixels, int width, int height) {

	SDL_Surface *ret;
	int y;

	// Create the surface
	ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

	// Set the surface's palette
	video.restoreSurfacePalette(ret);

	if (pixels) {

		// Upload pixel data to the surface
		if (SDL_MUSTLOCK(ret)) SDL_LockSurface(ret);

		for (y = 0; y < height; y++)
			memcpy(((unsigned char *)(ret->pixels)) + (ret->pitch * y),
				pixels + (width * y), width);

		if (SDL_MUSTLOCK(ret)) SDL_UnlockSurface(ret);

	}

	return ret;

}


Video::Video () {

	int count;

	screen = NULL;

	screenW = SW;
	screenH = SH;
#ifdef SCALE
	scaleFactor = 1;
#endif
#ifndef FULLSCREEN_ONLY
	fullscreen = false;
#endif

	// Generate the logical palette
	for (count = 0; count < 256; count++)
		logicalPalette[count].r = logicalPalette[count].g =
 			logicalPalette[count].b = count;

	currentPalette = logicalPalette;

	return;

}


bool Video::create (int width, int height) {

	screenW = width;
	screenH = height;

#ifdef SCALE
	if (canvas != screen) SDL_FreeSurface(canvas);
#endif

#if defined(WIZ) || defined(GP2X) || defined(DINGOO)
	screen = SDL_SetVideoMode(320, 240, 8, FULLSCREEN_FLAGS);
#else
	#ifdef FULLSCREEN_ONLY
	screen = SDL_SetVideoMode(screenW, screenH, 8, FULLSCREEN_FLAGS);
	#else
	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);
	#endif
#endif

	if (!screen) return false;


#ifdef SCALE
	// Check that the scale will fit in the current resolution
	while ( ((screenW/SW < scaleFactor) || (screenH/SH < scaleFactor)) && (scaleFactor > 1) ) {

		scaleFactor--;

	}

	if (scaleFactor > 1) {

		canvasW = screenW / scaleFactor;
		canvasH = screenH / scaleFactor;
		canvas = createSurface(NULL, canvasW, canvasH);

	} else {
#endif

		canvasW = screenW;
		canvasH = screenH;
		canvas = screen;

#ifdef SCALE
	}
#endif

#if !(defined(WIZ) || defined(GP2X))
	expose();
#endif


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

	return true;

}


void Video::setPalette (SDL_Color *palette) {

	// Make palette changes invisible until the next draw. Hopefully.
	clearScreen(SDL_MapRGB(screen->format, 0, 0, 0));
#ifndef SCALE
	SDL_Flip(screen);
#endif

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

	return;

}


SDL_Color* Video::getPalette () {

	return currentPalette;

}


void Video::changePalette (SDL_Color *palette, unsigned char first, unsigned int amount) {

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, first, amount);

	return;

}


void Video::restoreSurfacePalette (SDL_Surface* surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

	return;

}


int Video::getWidth () {

	return screenW;

}


int Video::getHeight () {

	return screenH;

}

#ifdef SCALE
int Video::getScaleFactor () {

	return scaleFactor;

}


void Video::setScaleFactor (int newScaleFactor) {

	scaleFactor = newScaleFactor;

	if (screen) create(screenW, screenH);

	return;

}
#endif

#ifndef FULLSCREEN_ONLY
bool Video::isFullscreen () {

	return fullscreen;

}
#endif


#ifndef FULLSCREEN_ONLY
void Video::flipFullscreen () {

	fullscreen = !fullscreen;

	SDL_ShowCursor(fullscreen? SDL_DISABLE: SDL_ENABLE);

	if (screen) create(screenW, screenH);

	return;

}
#endif


void Video::expose () {

	SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
	SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

	return;

}


void Video::flip (int mspf, PaletteEffect* paletteEffects) {

	SDL_Color shownPalette[256];

#ifdef SCALE
	if (canvas != screen) {

		// Copy everything that has been drawn so far
		scale(scaleFactor,
			screen->pixels, screen->pitch,
			canvas->pixels, canvas->pitch,
			screen->format->BytesPerPixel, canvas->w, canvas->h);

	}
#endif

	// Apply palette effects
	if (paletteEffects) {

		/* If the palette is being emulated, compile all palette changes and
		apply them all at once.
		If the palette is being used directly, apply all palette effects
		directly. */

		if (fakePalette) {

			memcpy(shownPalette, currentPalette, sizeof(SDL_Color) * 256);

			paletteEffects->apply(shownPalette, false, mspf);

			SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

		} else {

			paletteEffects->apply(shownPalette, true, mspf);

		}

	}

	// Show what has been drawn
	SDL_Flip(screen);

	return;

}


void clearScreen (int index) {

#if defined(WIZ) || defined(GP2X)
	// always 240 lines cleared to black
	memset(video->pixels, index, 320*240);
#else
	SDL_FillRect(canvas, NULL, index);
#endif

	return;

}


void drawRect (int x, int y, int width, int height, int index) {

	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;

	SDL_FillRect(canvas, &dst, index);

	return;

}

