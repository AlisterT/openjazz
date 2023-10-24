
/**
 *
 * @file video.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed util.c to util.cpp
 * - 13th July 2009: Created graphics.cpp from parts of util.cpp
 * - 26th July 2009: Renamed graphics.cpp to video.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Contains graphics utility functions.
 *
 */


#include "paletteeffects.h"
#include "video.h"

#ifdef SCALE
	#include <scalebit.h>
#endif

#include "setup.h"
#include "util.h"
#include "io/log.h"

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

	// Create the surface
	SDL_Surface *ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

	// Set the surface's palette
	video.restoreSurfacePalette(ret);

	if (pixels) {

		// Upload pixel data to the surface
		if (SDL_MUSTLOCK(ret)) SDL_LockSurface(ret);

		for (int y = 0; y < height; y++)
			memcpy(static_cast<unsigned char*>(ret->pixels) + (ret->pitch * y),
				pixels + (width * y), width);

		if (SDL_MUSTLOCK(ret)) SDL_UnlockSurface(ret);

	}

	return ret;

}


/**
 * Create the video output object.
 */
Video::Video () {

	screen = NULL;
	fakePalette = false;
	minW = maxW = screenW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = screenH = DEFAULT_SCREEN_HEIGHT;
#ifdef SCALE
	scaleFactor = 1;
#endif
	fullscreen = false;

	// Generate the logical palette
	for (int i = 0; i < 256; i++)
		logicalPalette[i].r = logicalPalette[i].g = logicalPalette[i].b = i;

	currentPalette = logicalPalette;

}


/**
 * Find the minimum and maximum horizontal and vertical resolutions.
 */
void Video::findResolutions () {

#ifdef NO_RESIZE
	minW = maxW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = DEFAULT_SCREEN_HEIGHT;
#else
	SDL_Rect **resolutions = SDL_ListModes(NULL, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	// All resolutions available, set to arbitrary limit
	if (resolutions == reinterpret_cast<SDL_Rect**>(-1)) {

		minW = SW;
		minH = SH;
		maxW = MAX_SCREEN_WIDTH;
		maxH = MAX_SCREEN_HEIGHT;

	} else {

		for (int i = 0; resolutions[i] != NULL; i++) {

			// Save largest resolution
			if (i == 0) {
				maxW = resolutions[i]->w;
				maxH = resolutions[i]->h;
			}

			// Save smallest resolution
			if (resolutions[i + 1] == NULL) {
				minW = resolutions[i]->w;
				minH = resolutions[i]->h;
			}

		}

		// Sanitize
		if (minW < SW) minW = SW;
		if (minH < SH) minH = SH;
		if (maxW > MAX_SCREEN_WIDTH) maxW = MAX_SCREEN_WIDTH;
		if (maxH > MAX_SCREEN_HEIGHT) maxH = MAX_SCREEN_HEIGHT;

	}
#endif

}


/**
 * Initialise video output.
 *
 * @param cfg Video Options
 *
 * @return Success
 */
bool Video::init (SetupOptions cfg) {

	fullscreen = cfg.fullScreen;

	if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

	findResolutions();

#ifdef SCALE
	if ((SW * cfg.videoScale <= cfg.videoWidth) &&
		(SH * cfg.videoScale <= cfg.videoHeight))
		scaleFactor = cfg.videoScale;
#endif

	if (!reset(cfg.videoWidth, cfg.videoHeight)) {

		LOG_FATAL("Could not set video mode: %s", SDL_GetError());

		return false;

	}

	setTitle(NULL);

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
bool Video::reset (int width, int height) {

#ifdef NO_RESIZE
	screenW = DEFAULT_SCREEN_WIDTH;
	screenH = DEFAULT_SCREEN_HEIGHT;
	fullscreen = true;
#else
	screenW = width;
	screenH = height;
#endif

#ifdef SCALE
	if (canvas != screen) SDL_FreeSurface(canvas);
#endif

	// If video mode is not valid reset to low default
	if (screenW < minW || screenW > maxW || screenH< minH || screenH > maxH) {
		screenW = minW;
		screenH = minH;
	}

	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

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

	} else
#endif
    {

		canvasW = screenW;
		canvasH = screenH;
		canvas = screen;

	}

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
	flip(0);

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

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

}


/**
 * Restores a surface's palette.
 *
 * @param surface Surface with a modified palette
 */
void Video::restoreSurfacePalette (SDL_Surface* surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

}


/**
 * Returns the minimum possible screen width.
 *
 * @return The minimum width
 */
int Video::getMinWidth () {

	return minW;

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
 * Returns the minimum possible screen height.
 *
 * @return The minimum height
 */
int Video::getMinHeight () {

	return minH;

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


/**
 * Sets the window title.
 *
 * @param the title or NULL, to use default
 */
void Video::setTitle (const char *title) {

	const char titleBase[] = "OpenJazz";
	char *windowTitle = NULL;
	int titleLen = strlen(titleBase) + 1;

	if (title != NULL) {

		titleLen = strlen(titleBase) + 3 + strlen(title) + 1;

	}

	windowTitle = new char[titleLen];

	strcpy(windowTitle, titleBase);

	if (title != NULL) {

		strcat(windowTitle, " - ");
		strcat(windowTitle, title);

	}

	SDL_WM_SetCaption(windowTitle, NULL);

	delete[] windowTitle;

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

		if (screen) reset(screenW, screenH);

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

}


/**
 * Update video based on a system event.
 *
 * @param event The system event. Events not affecting video will be ignored
 */
void Video::update (SDL_Event *event) {

#if !defined(FULLSCREEN_ONLY) || !defined(NO_RESIZE)
	switch (event->type) {

	#ifndef FULLSCREEN_ONLY
		case SDL_KEYDOWN:

			// If Alt + Enter has been pressed, switch between windowed and full-screen mode.
			if ((event->key.keysym.sym == SDLK_RETURN) &&
				(event->key.keysym.mod & KMOD_ALT)) {

				fullscreen = !fullscreen;

				if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

				findResolutions();

				reset(screenW, screenH);

				if (!fullscreen) SDL_ShowCursor(SDL_ENABLE);

			}

			break;
    #endif

    #ifndef NO_RESIZE
		case SDL_VIDEORESIZE:

			reset(event->resize.w, event->resize.h);

			break;
    #endif

		case SDL_VIDEOEXPOSE:

			expose();

			break;

	}
#endif

}


/**
 * Draw graphics to screen.
 *
 * @param mspf Ticks per frame
 * @param paletteEffects Palette effects to use
 * @param effectsStopped Whether the effects should be applied without advancing
 */
void Video::flip (int mspf, PaletteEffect* paletteEffects, bool effectsStopped) {

	SDL_Color shownPalette[256];

#ifdef SCALE
	if (canvas != NULL && canvas != screen) {

		// Copy everything that has been drawn so far
		if (setup.scale2x)
			scale(scaleFactor,
				screen->pixels, screen->pitch,
				canvas->pixels, canvas->pitch,
				screen->format->BytesPerPixel, canvas->w, canvas->h);
		else
			SDL_SoftStretch(canvas, NULL, screen, NULL);

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

			paletteEffects->apply(shownPalette, false, mspf, effectsStopped);

			SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

		} else {

			paletteEffects->apply(shownPalette, true, mspf, effectsStopped);

		}

	}

	// Show what has been drawn
	SDL_Flip(screen);

}


/**
 * Fill the screen with a colour.
 *
 * @param index Index of the colour to use
 */
void Video::clearScreen (int index) {

#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(GAMESHELL)
	// always 240 lines cleared to black
	memset(video.screen->pixels, index, 320*240);
#elif defined (_3DS)
	memset(video.screen->pixels, index, 400*240);
#else
	SDL_FillRect(canvas, NULL, index);
#endif

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

}
