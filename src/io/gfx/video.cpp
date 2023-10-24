
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

#if OJ_SDL2
	int surfaceFlag = 0; // flags are unused
#else
	int surfaceFlag = SDL_HWSURFACE;
#endif

	// Create the surface
	SDL_Surface *ret = SDL_CreateRGBSurface(surfaceFlag, width, height, 8, 0, 0, 0, 0);

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
	minW = maxW = screenW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = screenH = DEFAULT_SCREEN_HEIGHT;
#ifdef SCALE
	scaleFactor = 1;
#endif
	fullscreen = false;

	// Generate the logical palette
	for (int i = 0; i < MAX_PALETTE_COLORS; i++)
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

	// no need to sanitize
	return;
#elif OJ_SDL2
	SDL_DisplayMode mode;

	minW = SW;
	minH = SH;

	if (SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) >= 0) {
		maxW = mode.w;
		maxH = mode.h;
	} else {
		LOG_WARN("Could not query display mode, using defaults.");

		maxW = DEFAULT_SCREEN_WIDTH;
		maxH = DEFAULT_SCREEN_HEIGHT;
	}
#else
	SDL_Rect **resolutions = SDL_ListModes(NULL, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	// All resolutions available, set to arbitrary limit
	if (resolutions == reinterpret_cast<SDL_Rect**>(-1)) {
		LOG_DEBUG("No display mode limit found.");
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

	}
#endif

	// Sanitize
	if (minW < SW) minW = SW;
	if (minH < SH) minH = SH;
	if (maxW > MAX_SCREEN_WIDTH) maxW = MAX_SCREEN_WIDTH;
	if (maxH > MAX_SCREEN_HEIGHT) maxH = MAX_SCREEN_HEIGHT;

	LOG_TRACE("Allowing resolutions between %dx%d and %dx%d.", minW, minH, maxW, maxH);

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

#if OJ_SDL2
	window = SDL_CreateWindow("OpenJazz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SW, SH, fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: SDL_WINDOW_RESIZABLE);
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
 * Shared Deinitialisation code for reset() and deinit()
 *
 */
void Video::commonDeinit () {
	// canvas is used when scaling or built with SDL2
	if (canvas != screen && canvas) {
		SDL_FreeSurface(canvas);
		canvas = NULL;
	}

#if OJ_SDL2
	if(screen) {
		SDL_FreeSurface(screen);
		screen = NULL;
	}

	if(textureSurface) {
		SDL_FreeSurface(textureSurface);
		textureSurface = NULL;
	}

	if(texture) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
#endif
}

/**
 * Deinitialise video output.
 *
 */
void Video::deinit () {

	commonDeinit();

#if OJ_SDL2
	if(renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	if(window) {
		SDL_DestroyWindow(window);
		window = NULL;
	}
#endif

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

	commonDeinit();

	// If video mode is not valid reset to low default
	if (screenW < minW || screenW > maxW || screenH< minH || screenH > maxH) {
		LOG_WARN("Video mode invalid, resetting.");
		screenW = minW;
		screenH = minH;
	}

#if OJ_SDL2
	SDL_SetWindowSize(window, screenW, screenH);
	SDL_SetWindowFullscreen(window, fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: 0);
#else
	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);
	if (!screen) return false;
#endif

#ifdef SCALE
	// sanitize
	if (scaleFactor < 1) scaleFactor = 1;
	if (scaleFactor > 4) scaleFactor = 4;

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

#if OJ_SDL2
		canvas = createSurface(NULL, canvasW, canvasH);
#else
		canvas = screen;
#endif

	}

#if OJ_SDL2
	screen = SDL_CreateRGBSurfaceFrom(canvas->pixels, canvasW, canvasH, canvas->format->BitsPerPixel, canvas->pitch,
		                              canvas->format->Rmask, canvas->format->Gmask, canvas->format->Bmask, canvas->format->Amask);

	Uint32 format = SDL_PIXELFORMAT_RGB888;
	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer, &info) >= 0) {
		for (Uint32 i = 0; i < info.num_texture_formats; i++) {
			if (SDL_ISPIXELFORMAT_PACKED(info.texture_formats[i])) {
				format = info.texture_formats[i];
				break;
			}
		}
	}

	texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, canvasW, canvasH);
	if (!texture) {
		LOG_WARN("Could not create texture: %s\n", SDL_GetError());
		return false;
	}

	textureSurface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, canvasW, canvasH, SDL_BITSPERPIXEL(format), format);
	if (!textureSurface) {
		LOG_WARN("Could not create surface: %s\n", SDL_GetError());
		return false;
	}
#endif

#if !defined(WIZ) && !defined(GP2X)
	expose();
#endif

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

	changePalette(palette, 0, MAX_PALETTE_COLORS);
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

#if OJ_SDL2
	SDL_SetPaletteColors(screen->format->palette, palette, first, amount);
#else
	SDL_SetPalette(screen, SDL_PHYSPAL, palette, first, amount);
#endif

}


/**
 * Restores a surface's palette.
 *
 * @param surface Surface with a modified palette
 */
void Video::restoreSurfacePalette (SDL_Surface* surface) {

	setLogicalPalette(surface, logicalPalette, 0, MAX_PALETTE_COLORS);

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
	int titleLen = strlen(titleBase) + 1;

	if (title != NULL) {
		titleLen = strlen(titleBase) + 3 + strlen(title) + 1;
	}

	char *windowTitle = new char[titleLen];
	strcpy(windowTitle, titleBase);

	if (title != NULL) {
		strcat(windowTitle, " - ");
		strcat(windowTitle, title);
	}

#if OJ_SDL2
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

	setLogicalPalette(screen, logicalPalette, 0, MAX_PALETTE_COLORS);
	changePalette(currentPalette, 0, MAX_PALETTE_COLORS);

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

	#if !OJ_SDL2
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

}


/**
 * Draw graphics to screen.
 *
 * @param mspf Ticks per frame
 * @param paletteEffects Palette effects to use
 * @param effectsStopped Whether the effects should be applied without advancing
 */
void Video::flip (int mspf, PaletteEffect* paletteEffects, bool effectsStopped) {

	SDL_Color shownPalette[MAX_PALETTE_COLORS];

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
		// The palette is emulated, compile all palette changes and apply at once.
		memcpy(shownPalette, currentPalette, sizeof(SDL_Color) * MAX_PALETTE_COLORS);
		paletteEffects->apply(shownPalette, false, mspf, effectsStopped);
		changePalette(shownPalette, 0, MAX_PALETTE_COLORS);
	}

	// Show what has been drawn
#if OJ_SDL2
	SDL_BlitSurface(screen, NULL, textureSurface, NULL);
	SDL_UpdateTexture(texture, NULL, textureSurface->pixels, textureSurface->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
#else
	SDL_Flip(screen);
#endif

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

/**
 * Sets the Color key of provided surface.
 *
 * @param surface Surface to change
 * @param index Color index
 */
void enableColorKey (SDL_Surface* surface, unsigned int index) {

#if OJ_SDL2
	SDL_SetColorKey(surface, SDL_TRUE, index);
#else
	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, index);
#endif

}

/**
 * Returns the Color key of provided surface.
 *
 * @param surface Surface to query
 *
 * @return color index
 */
unsigned int getColorKey (SDL_Surface* surface) {

#if OJ_SDL2
	Uint32 key;

	if (SDL_GetColorKey(surface, &key) < 0) {
		LOG_WARN("Could not get Color Key: %s\n", SDL_GetError());
		return -1;
	}

	return key;
#else
	return surface->format->colorkey;
#endif

}

/**
 * Sets the palette colors of provided surface.
 *
 * @param surface Surface to change
 * @param palette Palette to copy colors from
 * @param start index of first color copy
 * @param length number of colors to copy
 */
void setLogicalPalette (SDL_Surface* surface, SDL_Color *palette, int start, int length) {

#if OJ_SDL2
	if (surface->format->palette) {
		SDL_SetPaletteColors(surface->format->palette, palette, start, length);
	} else
		LOG_WARN("Could not change palette on surface without palette.");
#else
	SDL_SetPalette(surface, SDL_LOGPAL, palette, start, length);
#endif

}
