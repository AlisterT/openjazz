
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

	SDL_Surface *ret;
	int y;

	// Create the surface
#if SDL_VERSION_ATLEAST(2, 0, 0)
	ret = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
#else
	ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);
#endif

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
 * Find the minimum and maximum horizontal and vertical resolutions.
 */
void Video::findResolutions () {

#ifdef NO_RESIZE
	minW = maxW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = DEFAULT_SCREEN_HEIGHT;
#elif SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DisplayMode mode;

	minW = maxW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = DEFAULT_SCREEN_HEIGHT;

	if (SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) >= 0) {
		maxW = mode.w;
		maxH = mode.h;
	}
#else
	SDL_Rect **resolutions;
	int count;

	resolutions = SDL_ListModes(NULL, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	// All resolutions available, set to arbitrary limit
	if (resolutions == (SDL_Rect **)(-1)) {

		minW = SW;
		minH = SH;
		maxW = MAX_SCREEN_WIDTH;
		maxH = MAX_SCREEN_HEIGHT;

	} else {

		for (count = 0; resolutions[count] != NULL; count++) {

			// Save largest resolution
			if (count == 0) {
				maxW = resolutions[count]->w;
				maxH = resolutions[count]->h;
			}

			// Save smallest resolution
			if (resolutions[count + 1] == NULL) {
				minW = resolutions[count]->w;
				minH = resolutions[count]->h;
			}

		}

		// Sanitize
		if (minW < SW) minW = SW;
		if (minH < SH) minH = SH;
		if (maxW > MAX_SCREEN_WIDTH) maxW = MAX_SCREEN_WIDTH;
		if (maxH > MAX_SCREEN_HEIGHT) maxH = MAX_SCREEN_HEIGHT;

	}
#endif

	return;
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	window = SDL_CreateWindow("OpenJazz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SW, SH, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	if (!window) {

		LOG_FATAL("Could not create window: %s", SDL_GetError());

		return false;

	}

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {

		LOG_FATAL("Could not create renderer: %s", SDL_GetError());

		return false;

	}

#endif

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

	screenW = width;
	screenH = height;

	if (canvas != screen) SDL_FreeSurface(canvas);
	canvas = NULL;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_FreeSurface(screen);
	screen = NULL;

	SDL_FreeSurface(textureSurface);
	textureSurface = NULL;

	SDL_DestroyTexture(texture);
	texture = NULL;
#endif

#ifdef NO_RESIZE
	screenW = DEFAULT_SCREEN_WIDTH;
	screenH = DEFAULT_SCREEN_HEIGHT;
	fullscreen = true;
#endif

	// If video mode is not valid reset to low default
	if (screenW < minW || screenW > maxW || screenH< minH || screenH > maxH) {
		screenW = minW;
		screenH = minH;
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)

	SDL_SetWindowSize(window, screenW, screenH);
	SDL_SetWindowFullscreen(window, fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: 0);

#else
	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	if (!screen) return false;

#endif

#ifdef SCALE
	// Check that the scale will fit in the current resolution
	while ( ((screenW/SW < scaleFactor) || (screenH/SH < scaleFactor)) && (scaleFactor > 1) ) {

		scaleFactor--;

	}

	if (scaleFactor > 1) {

		canvasW = screenW / scaleFactor;
		canvasH = screenH / scaleFactor;
		canvas = createSurface(NULL, canvasW, canvasH);
#if SDL_VERSION_ATLEAST(2, 0, 0)
		if (setup.scale2x && scaleFactor < 5)
			screen = createSurface(NULL, screenW, screenH);
#endif

	} else
#endif
    {

		canvasW = screenW;
		canvasH = screenH;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		canvas = createSurface(NULL, canvasW, canvasH);
#else
		canvas = screen;
#endif

	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (!screen) {
		screen = SDL_CreateRGBSurfaceFrom(canvas->pixels, canvas->w, canvas->h, canvas->format->BitsPerPixel, canvas->pitch,
		                                  canvas->format->Rmask, canvas->format->Gmask, canvas->format->Bmask, canvas->format->Amask);
	}

	Uint32 format = SDL_PIXELFORMAT_UNKNOWN;
	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer, &info) >= 0) {
		for (Uint32 i = 0; i < info.num_texture_formats; i++) {
			if (SDL_ISPIXELFORMAT_PACKED(info.texture_formats[i])) {
				format = info.texture_formats[i];
				break;
			}
		}
	}
	if (format == SDL_PIXELFORMAT_UNKNOWN)
		format = SDL_PIXELFORMAT_RGB888;

	texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, screen->w, screen->h);

	if (!texture) return false;

	textureSurface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, screen->w, screen->h, SDL_BITSPERPIXEL(format), format);

	if (!textureSurface) return false;
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
	clearScreen(SDL_MapRGB(canvas->format, 0, 0, 0));
	flip(0);

	changePalette(palette, 0, 256);
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetPaletteColors(screen->format->palette, palette, first, amount);
#else
	SDL_SetPalette(screen, SDL_PHYSPAL, palette, first, amount);
#endif

	return;

}


/**
 * Restores a surface's palette.
 *
 * @param surface Surface with a modified palette
 */
void Video::restoreSurfacePalette (SDL_Surface* surface) {

	setLogicalPalette(surface, logicalPalette, 0, 256);

	return;

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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetWindowTitle(window, windowTitle);
#else
	SDL_WM_SetCaption(windowTitle, NULL);
#endif

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

	setLogicalPalette(canvas, logicalPalette, 0, 256);
	changePalette(currentPalette, 0, 256);

	return;

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

    #if !SDL_VERSION_ATLEAST(2, 0, 0)
    #ifndef NO_RESIZE
		case SDL_VIDEORESIZE:

			reset(event->resize.w, event->resize.h);

			break;
    #endif

		case SDL_VIDEOEXPOSE:

			expose();

			break;
    #endif

	}
#endif

	return;

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
		if (setup.scale2x && scaleFactor < 5)
			scale(scaleFactor,
				screen->pixels, screen->pitch,
				canvas->pixels, canvas->pitch,
				screen->format->BytesPerPixel, canvas->w, canvas->h);
#if SDL_VERSION_ATLEAST(2, 0, 0)
		else
			SDL_SoftStretch(canvas, NULL, screen, NULL);
#endif

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

			changePalette(shownPalette, 0, 256);

		} else {

			paletteEffects->apply(shownPalette, true, mspf, effectsStopped);

		}

	}

	// Show what has been drawn
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_BlitSurface(screen, NULL, textureSurface, NULL);
	SDL_UpdateTexture(texture, NULL, textureSurface->pixels, textureSurface->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
#else
	SDL_Flip(screen);
#endif

	return;

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


void enableColorKey (SDL_Surface* surface, unsigned int index) {

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetColorKey(surface, SDL_TRUE, index);
#else
	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, index);
#endif

	return;

}

unsigned int getColorKey (SDL_Surface* surface) {

#if SDL_VERSION_ATLEAST(2, 0, 0)
	Uint32 key;

	if (SDL_GetColorKey(surface, &key) < 0)
		return -1;

	return key;
#else
	return surface->format->colorkey;
#endif

}

void setLogicalPalette (SDL_Surface* surface, SDL_Color *palette, int start, int length) {

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (surface->format->palette) {
		SDL_SetPaletteColors(surface->format->palette, palette, start, length);
	}
#else
	SDL_SetPalette(surface, SDL_LOGPAL, palette, start, length);
#endif

}
