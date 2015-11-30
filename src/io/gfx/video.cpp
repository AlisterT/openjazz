
/**
 *
 * @file video.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed util.c to util.cpp
 * 13th July 2009: Created graphics.cpp from parts of util.cpp
 * 26th July 2009: Renamed graphics.cpp to video.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2011 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Contains graphics utility functions.
 *
 */


#include "paletteeffects.h"
#include "video.h"

#ifdef SCALE
	#include "io/gfx/scale2x/scalebit.h"
#endif

#include "util.h"

#include <string.h>


/**
 * Creates a surface.
 *
 * @param pixels Pixel data to copy into the surface. Can be NULL.
 * @param width Width of the pixel data and of the surface to be created
 * @param height Height of the pixel data and of the surface to be created
 *
 * @return The completed surface
 */
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


/**
 * Create the video output object.
 */
Video::Video () {

	int count;

	screen = NULL;

#ifdef SCALE
	scaleFactor = 1;
#endif

	// Generate the logical palette
	for (count = 0; count < 256; count++)
		logicalPalette[count].r = logicalPalette[count].g =
 			logicalPalette[count].b = count;

	currentPalette = logicalPalette;

	return;

}


/**
 * Find the maximum horizontal and vertical resolutions.
 */
void Video::findMaxResolution () {

#if defined(CAANOO) ||defined(WIZ) || defined(GP2X) || defined(DINGOO)
	maxW = 320;
	maxH = 240;
#else
	SDL_Rect **resolutions;
	int count;

	resolutions = SDL_ListModes(NULL, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	if (resolutions == (SDL_Rect **)(-1)) {

		maxW = MAX_SW;
		maxH = MAX_SH;

	} else {

		maxW = SW;
		maxH = SH;

		for (count = 0; resolutions[count] != NULL; count++) {

			if (resolutions[count]->w > maxW) maxW = resolutions[count]->w;
			if (resolutions[count]->h > maxH) maxH = resolutions[count]->h;

		}

		if (maxW > MAX_SW) maxW = MAX_SW;
		if (maxH > MAX_SH) maxH = MAX_SH;
	}
#endif

	return;
}


/**
 * Initialise video output.
 *
 * @param width Width of the window or screen
 * @param height Height of the window or screen
 * @param startFullscreen Whether or not to start in full-screen mode
 *
 * @return Success
 */
bool Video::init (int width, int height, bool startFullscreen) {

	fullscreen = startFullscreen;

	if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

	if (!resize(width, height)) {

		logError("Could not set video mode", SDL_GetError());

		return false;

	}

	SDL_WM_SetCaption("OpenJazz", NULL);

	findMaxResolution();

	return true;

}


/**
 * Sets the size of the video window or the resolution of the screen.
 *
 * @param width New width of the window or screen
 * @param height New height of the window or screen
 *
 * @return Success
 */
bool Video::resize (int width, int height) {

	screenW = width;
	screenH = height;

#ifdef SCALE
	if (canvas != screen) SDL_FreeSurface(canvas);
#endif

#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(DINGOO)
	screen = SDL_SetVideoMode(320, 240, 8, FULLSCREEN_FLAGS);
#else
	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);
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

#if !defined(WIZ) && !defined(GP2X)
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
	/// @todo Find a better way to determine if palette is emulated
	fakePalette = true;

	return true;

}


/**
 * Sets the display palette.
 *
 * @param palette The new palette
 */
void Video::setPalette (SDL_Color *palette) {

	// Make palette changes invisible until the next draw. Hopefully.
	clearScreen(SDL_MapRGB(screen->format, 0, 0, 0));
	flip(0, NULL);

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

	return;

}


/**
 * Returns the current display palette.
 *
 * @return The current display palette
 */
SDL_Color* Video::getPalette () {

	return currentPalette;

}


/**
 * Sets some colours of the display palette.
 *
 * @param palette The palette containing the new colours
 * @param first The index of the first colour in both the display palette and the specified palette
 * @param amount The number of colours
 */
void Video::changePalette (SDL_Color *palette, unsigned char first, unsigned int amount) {

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, first, amount);

	return;

}


/**
 * Restores a surface's palette.
 *
 * @param surface Surface with a modified palette
 */
void Video::restoreSurfacePalette (SDL_Surface* surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

	return;

}


/**
 * Returns the maximum possible screen width.
 *
 * @return The maximum width
 */
int Video::getMaxWidth () {

	return maxW;

}


/**
 * Returns the maximum possible screen height.
 *
 * @return The maximum height
 */
int Video::getMaxHeight () {

	return maxH;

}


/**
 * Returns the current width of the window or screen.
 *
 * @return The width
 */
int Video::getWidth () {

	return screenW;

}


/**
 * Returns the current height of the window or screen.
 *
 * @return The height
 */
int Video::getHeight () {

	return screenH;

}


#ifdef SCALE
/**
 * Returns the current scaling factor.
 *
 * @return The scaling factor
 */
int Video::getScaleFactor () {

	return scaleFactor;

}


/**
 * Sets the scaling factor.
 *
 * @param newScaleFactor The new scaling factor
 */
int Video::setScaleFactor (int newScaleFactor) {

	if ((SW * newScaleFactor <= screenW) && (SH * newScaleFactor <= screenH)) {

		scaleFactor = newScaleFactor;

		if (screen) resize(screenW, screenH);

	}

	return scaleFactor;

}
#endif

#ifndef FULLSCREEN_ONLY
/**
 * Determines whether or not full-screen mode is being used.
 *
 * @return Whether or not full-screen mode is being used
 */
bool Video::isFullscreen () {

	return fullscreen;

}
#endif


/**
 * Refresh display palette.
 */
void Video::expose () {

	SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
	SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

	return;

}


/**
 * Update video based on a system event.
 *
 * @param event The system event. Events not affecting video will be ignored
 */
void Video::update (SDL_Event *event) {

#ifndef FULLSCREEN_ONLY
	switch (event->type) {

		case SDL_KEYDOWN:

			// If Alt + Enter has been pressed, switch between windowed and full-screen mode.
			if ((event->key.keysym.sym == SDLK_RETURN) &&
				(event->key.keysym.mod & KMOD_ALT)) {

				fullscreen = !fullscreen;

				if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

				resize(screenW, screenH);

				if (!fullscreen) SDL_ShowCursor(SDL_ENABLE);

				findMaxResolution();

			}

			break;

		case SDL_VIDEORESIZE:

			resize(event->resize.w, event->resize.h);

			break;

		case SDL_VIDEOEXPOSE:

			expose();

			break;

	}
#endif

	return;

}


/**
 * Draw graphics to screen.
 *
 * @param mspf Ticks per frame
 * @param paletteEffects Palette effects to use
 */
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


/**
 * Fill the screen with a colour.
 *
 * @param index Index of the colour to use
 */
void Video::clearScreen (int index) {

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	// always 240 lines cleared to black
	memset(video.screen->pixels, index, 320*240);
#else
	SDL_FillRect(canvas, NULL, index);
#endif

	return;

}


/**
 * Fill a specified rectangle of the screen with a colour.
 *
 * @param x X-coordinate of the left side of the rectangle
 * @param y Y-coordinate of the top of the rectangle
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @param index Index of the colour to use
 */
void drawRect (int x, int y, int width, int height, int index) {

	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;

	SDL_FillRect(canvas, &dst, index);

	return;

}

