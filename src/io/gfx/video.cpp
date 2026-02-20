
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
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2026 Carsten Teibes
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
 * @param pixels Pixel data to copy into the surface. Can be nullptr.
 * @param width Width of the pixel data and of the surface to be created
 * @param height Height of the pixel data and of the surface to be created
 *
 * @return The completed surface
 */
SDL_Surface* Video::createSurface (const unsigned char * pixels, int width, int height) {

	// Create the surface
	SDL_Surface *ret = nullptr;
#if OJ_SDL3
	ret = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
	if(ret) {
		SDL_Palette *pal = SDL_CreateSurfacePalette(ret);
		if(!pal) {
			LOG_FATAL("Could not create surface palette: %s", SDL_GetError());
			return nullptr;
		}
	}
#elif OJ_SDL2
	ret = SDL_CreateRGBSurfaceWithFormat(0, width, height, 0, SDL_PIXELFORMAT_INDEX8);
#else
	ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);
#endif

	if(!ret) {
		LOG_FATAL("Could not create surface: %s", SDL_GetError());
		return nullptr;
	}

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

void Video::destroySurface (SDL_Surface *surface) {
	if (!surface)
		return;

#if OJ_SDL3
	SDL_DestroySurface(surface);
#else
	SDL_FreeSurface(surface);
#endif
	surface = nullptr;
}

void Video::setClipRect (SDL_Surface *surface, const SDL_Rect *rect) {
#if OJ_SDL3
	SDL_SetSurfaceClipRect(surface, rect);
#else
	SDL_SetClipRect(surface, rect);
#endif
}

static void showCursor(bool enable) {
#if OJ_SDL3
	if(enable) {
		SDL_ShowCursor();
	} else {
		SDL_HideCursor();
	}
#else
	SDL_ShowCursor(enable? SDL_ENABLE: SDL_DISABLE);
#endif
}


/**
 * Create the video output object.
 */
Video::Video () :
#if OJ_SDL3 || OJ_SDL2
	window(nullptr), renderer(nullptr), texture(nullptr),
#endif
#if OJ_SDL2
	textureSurface(nullptr),
#endif
	screen(nullptr), scaleFactor(MIN_SCALE), scaleMethod(scalerType::None),
	fullscreen(false), isPlayingMovie(false) {

	minW = maxW = screenW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = screenH = DEFAULT_SCREEN_HEIGHT;

	// Generate the logical palette
#if OJ_SDL3
	logicalPalette = SDL_CreatePalette(MAX_PALETTE_COLORS);
	for (int i = 0; i < MAX_PALETTE_COLORS; i++) {
		logicalPalette->colors[i].r = logicalPalette->colors[i].g =
			logicalPalette->colors[i].b = i;
		logicalPalette->colors[i].a = 0xFF;
	}

	texturePalette = SDL_CreatePalette(MAX_PALETTE_COLORS);
	currentPalette = logicalPalette->colors;
#else
	for (int i = 0; i < MAX_PALETTE_COLORS; i++) {
		logicalPalette[i].r = logicalPalette[i].g = logicalPalette[i].b = i;
	#if OJ_SDL2
		logicalPalette[i].a = 0xFF;
	#endif
	}

	currentPalette = logicalPalette;
#endif
}


/**
 * Find the minimum and maximum horizontal and vertical resolutions.
 */
void Video::findResolutions () {

#ifdef NO_RESIZE
	minW = maxW = DEFAULT_SCREEN_WIDTH;
	minH = maxH = DEFAULT_SCREEN_HEIGHT;

	LOG_DEBUG("Using fixed resolution %dx%d.",
		DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);

	// no need to sanitize
	return;
#elif OJ_SDL3 || OJ_SDL2
	minW = SW;
	minH = SH;

	bool queryOk = false;
	#if OJ_SDL3
	const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(SDL_GetDisplayForWindow(window));
	queryOk = mode != nullptr;
	if(queryOk) {
		maxW = mode->w;
		maxH = mode->h;
	}
	#else
	SDL_DisplayMode mode;
	queryOk = SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &mode) == 0;
	if(queryOk) {
		maxW = mode.w;
		maxH = mode.h;
	}
	#endif
	if (!queryOk) {
		LOG_WARN("Could not query display mode, using defaults.");

		maxW = DEFAULT_SCREEN_WIDTH;
		maxH = DEFAULT_SCREEN_HEIGHT;
	}
#else
	SDL_Rect **resolutions = SDL_ListModes(nullptr, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	// All resolutions available, set to arbitrary limit
	if (resolutions == reinterpret_cast<SDL_Rect**>(-1)) {
		LOG_DEBUG("No display mode limit found.");
		minW = SW;
		minH = SH;
		maxW = MAX_SCREEN_WIDTH;
		maxH = MAX_SCREEN_HEIGHT;
	} else {

		for (int i = 0; resolutions[i] != nullptr; i++) {

			// Save largest resolution
			if (i == 0) {
				maxW = resolutions[i]->w;
				maxH = resolutions[i]->h;
			}

			// Save smallest resolution
			if (resolutions[i + 1] == nullptr) {
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

	// The window stays for whole runtime, as it is needed for
	// e.g. determining desktop resolution
#if OJ_SDL3 || OJ_SDL2
	#if OJ_SDL3
	window = SDL_CreateWindow("OpenJazz", SW, SH, fullscreen? SDL_WINDOW_FULLSCREEN: SDL_WINDOW_RESIZABLE);
	#elif OJ_SDL2
	window = SDL_CreateWindow("OpenJazz", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, SW, SH,
		fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: SDL_WINDOW_RESIZABLE);
	#endif
	if (!window) {
		LOG_FATAL("Could not create window: %s", SDL_GetError());
		return false;
	}
#endif

	if (fullscreen) showCursor(false);

	findResolutions();

#ifdef SCALE
	if ((SW * cfg.videoScale <= cfg.videoWidth) &&
		(SH * cfg.videoScale <= cfg.videoHeight))
		scaleFactor = cfg.videoScale;

	scaleMethod = cfg.scaleMethod;
#endif

	if (!reset(cfg.videoWidth, cfg.videoHeight)) {
		LOG_FATAL("Could not set video mode: %s", SDL_GetError());
		return false;
	}

	setTitle(nullptr);

	return true;

}


/**
 * Shared Deinitialisation code for reset() and deinit()
 *
 */
void Video::commonDeinit () {
	// canvas is used when scaling or built with SDL2
	if (canvas && canvas != screen)
		destroySurface(canvas);

#if OJ_SDL2
	if(textureSurface)
		destroySurface(textureSurface);
#endif

#if OJ_SDL3 || OJ_SDL2
	if(screen)
		destroySurface(screen);

	if(texture) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}

	if(renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
#endif
}

/**
 * Deinitialise video output.
 *
 */
void Video::deinit () {

	commonDeinit();

#if OJ_SDL3 || OJ_SDL2
	if(window) {
		SDL_DestroyWindow(window);
		window = nullptr;
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
	(void)width;
	(void)height;
	screenW = DEFAULT_SCREEN_WIDTH;
	screenH = DEFAULT_SCREEN_HEIGHT;
	fullscreen = true;
#else
	screenW = width;
	screenH = height;
#endif

	commonDeinit();

	// If video mode is not valid reset to low default
	if (screenW < minW || screenW > maxW || screenH < minH || screenH > maxH) {
		LOG_WARN("Video mode invalid, resetting.");
		screenW = minW;
		screenH = minH;
	}

#if OJ_SDL3
	renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer) {
		LOG_FATAL("Could not create renderer: %s", SDL_GetError());
		return false;
	}

	SDL_SetWindowSize(window, screenW, screenH);
	SDL_SetWindowFullscreen(window, fullscreen? SDL_WINDOW_FULLSCREEN: 0);
	screen = createSurface(nullptr, screenW, screenH);
	if (!screen) {
		LOG_FATAL("Could not create screen surface: %s", SDL_GetError());
		return false;
	}
#elif OJ_SDL2
	// Let the renderer scale the texture
	switch(scaleMethod) {
		case scalerType::Bilinear:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
			break;
		default:
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
			break;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		LOG_FATAL("Could not create renderer: %s", SDL_GetError());
		return false;
	}

	SDL_SetWindowSize(window, screenW, screenH);
	SDL_SetWindowFullscreen(window, fullscreen? SDL_WINDOW_FULLSCREEN_DESKTOP: 0);
#else
	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);
	if (!screen) return false;
#endif

#ifdef SCALE
	// sanitize
	scaleFactor = CLAMP(scaleFactor, MIN_SCALE, MAX_SCALE);

	// Check that the scale will fit in the current resolution
	while ( ((screenW/SW < scaleFactor) || (screenH/SH < scaleFactor))
		&& (scaleFactor > MIN_SCALE) ) {
		scaleFactor--;
	}

	if (scaleFactor > MIN_SCALE) {
		// Do scaling
		canvasW = screenW / scaleFactor;
		canvasH = screenH / scaleFactor;
		canvas = createSurface(nullptr, canvasW, canvasH);

	} else
#endif
	{
		// No scaling
		canvasW = screenW;
		canvasH = screenH;

#if OJ_SDL3 || OJ_SDL2
		canvas = createSurface(nullptr, canvasW, canvasH);
#else
		canvas = screen;
#endif

	}

#if OJ_SDL3 || OJ_SDL2
	int renderW = canvasW;
	int renderH = canvasH;

#if SCALE
	// Do prescaling (scaleX, hqx)
	if(scaleMethod != scalerType::None && scaleMethod != scalerType::Bilinear) {
		renderW = screenW;
		renderH = screenH;
	}
#endif

	#if OJ_SDL3
	SDL_SetRenderLogicalPresentation(renderer, renderW, renderH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	const char *name = SDL_GetRendererName(renderer);
	LOG_TRACE("Using '%s' pixel format with '%s' renderer", SDL_GetPixelFormatName(SDL_PIXELFORMAT_INDEX8), name);

	// Create texture for pixel data upload (8 bit)
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_STREAMING, renderW, renderH);
	if (!texture) {
		LOG_ERROR("Could not create texture: %s", SDL_GetError());
		return false;
	}
	if(!SDL_SetTexturePalette(texture, texturePalette)) {
		LOG_ERROR("Could not set texture palette: %s", SDL_GetError());
		return false;
	}
	// Let the renderer scale the texture
	switch(scaleMethod) {
		case scalerType::Bilinear:
			SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
			break;
		default:
			SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
			break;
	}
	#elif OJ_SDL2
	SDL_RenderSetLogicalSize(renderer, renderW, renderH);

	Uint32 format = SDL_MasksToPixelFormatEnum(canvas->format->BitsPerPixel,
		canvas->format->Rmask, canvas->format->Gmask, canvas->format->Bmask, canvas->format->Amask);
	LOG_TRACE("Screen surface is using '%s' pixel format", SDL_GetPixelFormatName(format));
	screen = SDL_CreateRGBSurfaceWithFormatFrom(canvas->pixels, canvasW, canvasH,
		canvas->format->BitsPerPixel, canvas->pitch, format);

	// Find a suitable renderer/texture format
	format = SDL_PIXELFORMAT_RGB888;
	SDL_RendererInfo info;
	if (SDL_GetRendererInfo(renderer, &info) >= 0) {
		for (Uint32 i = 0; i < info.num_texture_formats; i++) {
			if (SDL_ISPIXELFORMAT_PACKED(info.texture_formats[i])) {
				format = info.texture_formats[i];
				break;
			}
		}
	}
	LOG_TRACE("Using '%s' pixel format with '%s' renderer", SDL_GetPixelFormatName(format), info.name);

	// Create texture with compatible surface for pixel data upload (likely 32 bit)
	texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, renderW, renderH);
	if (!texture) {
		LOG_ERROR("Could not create texture: %s", SDL_GetError());
		return false;
	}

	textureSurface = SDL_CreateRGBSurfaceWithFormat(0, renderW, renderH, SDL_BITSPERPIXEL(format), format);
	if (!textureSurface) {
		LOG_ERROR("Could not create surface: %s", SDL_GetError());
		return false;
	}
	#endif
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
#if OJ_SDL3
	const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(canvas->format);
	clearScreen(SDL_MapRGB(details, nullptr, 0, 0, 0));
#else
	clearScreen(SDL_MapRGB(screen->format, 0, 0, 0));
#endif
	flip(0);

	changePalette(palette, 0, MAX_PALETTE_COLORS);
	currentPalette = palette;

}

/**
 * Sets some colours of the display palette.
 *
 * @param palette The palette containing the new colours
 * @param first The index of the first colour in both the display palette and the specified palette
 * @param amount The number of colours
 */
void Video::changePalette (SDL_Color *palette, unsigned char first, unsigned int amount) {
#if OJ_SDL3 || OJ_SDL2
	setSurfacePalette(screen, palette, first, amount);
	#if OJ_SDL3
	if(!SDL_SetPaletteColors(texturePalette, palette, first, amount)) {
		LOG_WARN("Could not set texture palette colors: %s", SDL_GetError());
	}
	#endif
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
#if OJ_SDL3
	setSurfacePalette(surface, logicalPalette->colors, 0, MAX_PALETTE_COLORS);
#else
	setSurfacePalette(surface, logicalPalette, 0, MAX_PALETTE_COLORS);
#endif
}


/**
 * Sets the window title.
 *
 * @param title the title or nullptr, to use default
 */
void Video::setTitle (const char *title) {

	constexpr const char *titleBase = "OpenJazz";
	int titleLen = strlen(titleBase) + 1;

	if (title != nullptr) {
		titleLen = strlen(titleBase) + 3 + strlen(title) + 1;
	}

	char *windowTitle = new char[titleLen];
	strcpy(windowTitle, titleBase);

	if (title != nullptr) {
		strcat(windowTitle, " - ");
		strcat(windowTitle, title);
	}

#if OJ_SDL3 || OJ_SDL2
	SDL_SetWindowTitle(window, windowTitle);
#else
	SDL_WM_SetCaption(windowTitle, nullptr);
#endif

	delete[] windowTitle;

}


/**
 * Sets the scaling factor and method.
 *
 * @param newScaleFactor The new scaling factor
 * @param newScaleMethod The new scaling method
 */
void Video::setScaling (int newScaleFactor, scalerType newScaleMethod) {
	bool reset_needed = false;

	if ((SW * newScaleFactor <= screenW) && (SH * newScaleFactor <= screenH)) {
		reset_needed = (scaleFactor != newScaleFactor);
		scaleFactor = newScaleFactor;
	}

	if(scaleMethod != newScaleMethod) {
		scaleMethod = newScaleMethod;
		reset_needed = true;
	}

	if(reset_needed) {
		if (screen)
			reset(screenW, screenH);
	}
}


/**
 * Refresh display palette.
 */
void Video::expose () {
#if OJ_SDL3
	setSurfacePalette(screen, logicalPalette->colors, 0, MAX_PALETTE_COLORS);
#else
	setSurfacePalette(screen, logicalPalette, 0, MAX_PALETTE_COLORS);
#endif
	changePalette(currentPalette, 0, MAX_PALETTE_COLORS);
}


/**
 * Update video based on a system event.
 *
 * @param event The system event. Events not affecting video will be ignored
 */
void Video::update (SDL_Event *event) {
#if !defined(FULLSCREEN_ONLY) || !defined(NO_RESIZE)
	auto switchFullscreen = [&] () {
		fullscreen = !fullscreen;
		if (fullscreen) showCursor(false);

		findResolutions();

		reset(screenW, screenH);
		if (!fullscreen) showCursor(true);
	};

	switch (event->type) {

	#ifndef FULLSCREEN_ONLY
		#if OJ_SDL3
		case SDL_EVENT_KEY_DOWN:
			// If Alt + Enter has been pressed, switch between windowed and full-screen mode.
			if ((event->key.key == SDLK_RETURN) && (event->key.mod & SDL_KMOD_ALT)) {
				switchFullscreen();
			}
		#else
		case SDL_KEYDOWN:
			// If Alt + Enter has been pressed, switch between windowed and full-screen mode.
			if ((event->key.keysym.sym == SDLK_RETURN) &&
				(event->key.keysym.mod & KMOD_ALT)) {
				switchFullscreen();
			}

		#endif

			break;
	#endif

	#if !OJ_SDL3 && !OJ_SDL2
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
#else
	(void)event;
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

#if defined(SCALE) && !OJ_SDL2 && !OJ_SDL3
	// for SDL1.2 scale 8 bit canvas surface
	if (canvas != nullptr && canvas != screen) {

		// Copy everything that has been drawn so far
		if (scaleMethod == scalerType::Scale2x)
			scale(scaleFactor,
				screen->pixels, screen->pitch,
				canvas->pixels, canvas->pitch,
				screen->format->BytesPerPixel, canvas->w, canvas->h);
		else
			SDL_SoftStretch(canvas, nullptr, screen, nullptr);

	}
#endif

	// Apply palette effects
	if (paletteEffects) {
		// The palette is emulated, compile all palette changes and apply at once.
		memcpy(shownPalette, currentPalette, sizeof(SDL_Color) * MAX_PALETTE_COLORS);
		paletteEffects->apply(shownPalette, false, mspf, effectsStopped);
		changePalette(shownPalette, 0, MAX_PALETTE_COLORS);
	}

#if OJ_SDL3
	// Show what has been drawn
	SDL_UpdateTexture(texture, nullptr, canvas->pixels, canvas->pitch);
	SDL_RenderClear(renderer);
	if (isPlayingMovie) {
		SDL_FRect src = {0, 0, static_cast<float>(canvasW), static_cast<float>(canvasH)};
		SDL_RenderTexture(renderer, texture, &src, nullptr);
	} else {
		SDL_RenderTexture(renderer, texture, nullptr, nullptr);
	}
	SDL_RenderPresent(renderer);
#elif OJ_SDL2
#if defined(SCALE)
	if(scaleFactor > MIN_SCALE && scaleMethod == scalerType::Scale2x) {
		// scale 32 bit screen surface
		scale(scaleFactor,
			textureSurface->pixels, textureSurface->pitch,
			screen->pixels, screen->pitch,
			screen->format->BytesPerPixel, screen->w, screen->h);
	} else if (scaleFactor > MIN_SCALE && scaleMethod == scalerType::hqx) {
			// TODO
	} else {
#endif
		// copy unscaled
		SDL_BlitSurface(screen, nullptr, textureSurface, nullptr);
#if defined(SCALE)
	}
#endif

	// Show what has been drawn
	SDL_UpdateTexture(texture, nullptr, textureSurface->pixels, textureSurface->pitch);
	SDL_RenderClear(renderer);

	if (isPlayingMovie) {
		SDL_Rect src = {0, 0, canvasW, canvasH};
		SDL_RenderCopy(renderer, texture, &src, nullptr);
	} else {
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	}
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
	memset(screen->pixels, index, 320*240);
#elif defined (__3DS__)
	memset(screen->pixels, index, 400*240);
#elif OJ_SDL3
	SDL_FillSurfaceRect(canvas, nullptr, index);
#else
	SDL_FillRect(canvas, nullptr, index);
#endif

}


/**
 * Sets up scaling for movie mode.
 *
 * @param status Whether or not movie mode will be enabled
 */
void Video::moviePlayback (bool status) {
#if OJ_SDL3 || OJ_SDL2
	static int movieW = SW;
	static int movieH = SH;

	if(isPlayingMovie == status)
		return;

	isPlayingMovie = status;

	if (isPlayingMovie) {
		// save size
		movieW = canvasW;
		movieH = canvasH;

		canvasW = SW;
		canvasH = SH;
	} else {
		// reset
		canvasW = movieW;
		canvasH = movieH;
	}
	#if OJ_SDL3
	SDL_SetRenderLogicalPresentation(renderer, canvasW, canvasH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	#else
	SDL_RenderSetLogicalSize(renderer, canvasW, canvasH);
	#endif
#else
	(void)status;
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
 * @param fill Whether to fill or only color the borders
 */
void Video::drawRect (int x, int y, int width, int height, int index, bool fill) {
	SDL_Rect dst[4];

	for(int i = 0; i < 4; i++) {
		dst[i].x = x;
		dst[i].y = y;
		dst[i].w = width;
		dst[i].h = height;
	}

	if (fill) {
#if OJ_SDL3
		SDL_FillSurfaceRect(canvas, &dst[0], index);
#else
		SDL_FillRect(canvas, &dst[0], index);
#endif
	} else {
		// left
		dst[0].w = 1;
		// right
		dst[1].x = x + width - 1;
		dst[1].w = 1;
		// top
		dst[2].h = 1;
		// bottom
		dst[3].y = y + height - 1;
		dst[3].h = 1;
		// draw each border
#if OJ_SDL3
		SDL_FillSurfaceRects(canvas, dst, 4, index);
#elif OJ_SDL2
		SDL_FillRects(canvas, dst, 4, index);
#else
		SDL_FillRect(canvas, &dst[0], index);
		SDL_FillRect(canvas, &dst[1], index);
		SDL_FillRect(canvas, &dst[2], index);
		SDL_FillRect(canvas, &dst[3], index);
#endif
	}
}

/**
 * Sets the Color key of provided surface.
 *
 * @param surface Surface to change
 * @param index Color index
 */
void Video::enableColorKey (SDL_Surface* surface, unsigned int index) {

#if OJ_SDL3
	SDL_SetSurfaceColorKey(surface, true, index);
#elif OJ_SDL2
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
unsigned int Video::getColorKey (SDL_Surface* surface) {

#if OJ_SDL3 || OJ_SDL2
	Uint32 key;
	bool queryOk = false;

	#if OJ_SDL3
	queryOk = SDL_GetSurfaceColorKey(surface, &key);
	#else
	queryOk = SDL_GetColorKey(surface, &key) == 0;
	#endif

	if (!queryOk) {
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
void Video::setSurfacePalette (SDL_Surface* surface, SDL_Color *palette, int start, int length) {
#if OJ_SDL3
	SDL_Palette *surfacePalette = SDL_GetSurfacePalette(surface);
	if(!surfacePalette) {
		LOG_WARN("Could not set palette on surface without palette.");
		return;
	}

	if(!SDL_SetPaletteColors(surfacePalette, palette, start, length)) {
		LOG_WARN("Could not set palette colors: %s", SDL_GetError());
		return;
	}
#elif OJ_SDL2
	if (surface->format->palette) {
		SDL_SetPaletteColors(surface->format->palette, palette, start, length);
	} else
		LOG_WARN("Could not change palette on surface without palette.");
#else
	SDL_SetPalette(surface, SDL_LOGPAL, palette, start, length);
#endif

}
