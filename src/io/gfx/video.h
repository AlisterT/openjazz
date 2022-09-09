
/**
 *
 * @file video.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 13th July 2009: Created graphics.h from parts of OpenJazz.h
 * - 26th July 2009: Renamed graphics.h to video.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#ifndef _VIDEO_H
#define _VIDEO_H

#include "setup.h"
#include "paletteeffects.h"

#include <SDL.h>


// Constants

// Original screen dimensions
#define SW 320
#define SH 200

#define MIN_SCALE 1
#ifdef SCALE
	#define MAX_SCALE 4
#else
	#define MAX_SCALE 1
#endif

// Maximum screen dimensions
#define MAX_SCREEN_WIDTH (32 * 256 * MAX_SCALE)
#define MAX_SCREEN_HEIGHT (32 * 64 * MAX_SCALE)

#define WINDOWED_FLAGS (SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)

#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(GAMESHELL)
	#define DEFAULT_SCREEN_WIDTH 320
	#define DEFAULT_SCREEN_HEIGHT 240

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#elif defined(DINGOO)
	#define DEFAULT_SCREEN_WIDTH 320
	#define DEFAULT_SCREEN_HEIGHT 240

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#define FULLSCREEN_FLAGS 0
#elif defined(PSP)
	#define DEFAULT_SCREEN_WIDTH 480
	#define DEFAULT_SCREEN_HEIGHT 272

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#elif defined(_3DS)
	#define DEFAULT_SCREEN_WIDTH 400
	#define DEFAULT_SCREEN_HEIGHT 240

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#define FULLSCREEN_FLAGS (SDL_SWSURFACE | SDL_TOPSCR | SDL_CONSOLEBOTTOM)
#elif defined(WII)
	#define DEFAULT_SCREEN_WIDTH 640
	#define DEFAULT_SCREEN_HEIGHT 480

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#elif defined(__vita__)
	#define DEFAULT_SCREEN_WIDTH 960
	#define DEFAULT_SCREEN_HEIGHT 540

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_HWSURFACE)
#elif SDL_VERSION_ATLEAST(2, 0, 0)
	#define DEFAULT_SCREEN_WIDTH SW
	#define DEFAULT_SCREEN_HEIGHT SH

	#undef WINDOWED_FLAGS
	#define WINDOWED_FLAGS (SDL_WINDOW_RESIZABLE)
	#define FULLSCREEN_FLAGS (SDL_WINDOW_FULLSCREEN_DESKTOP)
#else
	#define DEFAULT_SCREEN_WIDTH SW
	#define DEFAULT_SCREEN_HEIGHT SH

	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)
#endif

// Time interval
#define T_MENU_FRAME 20


// Class

/// Video output
class Video {

	private:
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_Window* window; ///< Output window
		SDL_Renderer* renderer; ///< Output renderer
		SDL_Texture* texture; ///< Output texture
		SDL_Surface* textureSurface;
#endif
		SDL_Surface* screen; ///< Output surface

		// Palettes
		SDL_Color*   currentPalette; ///< Current palette
		SDL_Color    logicalPalette[256]; ///< Logical palette (greyscale)
		bool         fakePalette; ///< Whether or not the palette mode is being emulated

		int          minW; ///< Smallest possible width
		int          maxW; ///< Largest possible width
		int          minH; ///< Smallest possible height
		int          maxH; ///< Largest possible height
		int          screenW; ///< Real width
		int          screenH; ///< Real height
#ifdef SCALE
		int          scaleFactor; ///< Scaling factor
#endif
		bool         fullscreen; ///< Full-screen mode

		void findResolutions ();
		void expose          ();

	public:
		Video ();

		bool       init                  (SetupOptions cfg);

		bool       reset                 (int width, int height);

		void       setPalette            (SDL_Color *palette);
		SDL_Color* getPalette            ();
		void       changePalette         (SDL_Color *palette, unsigned char first, unsigned int amount);
		void       restoreSurfacePalette (SDL_Surface *surface);

		int        getMinWidth           ();
		int        getMaxWidth           ();
		int        getMinHeight          ();
		int        getMaxHeight          ();
		int        getWidth              ();
		int        getHeight             ();
		void       setTitle              (const char *title);
#ifdef SCALE
		int        getScaleFactor        ();
		int        setScaleFactor        (int newScaleFactor);
#endif
#ifndef FULLSCREEN_ONLY
		bool       isFullscreen          ();
#endif

		void       update                (SDL_Event *event);
		void       flip                  (int mspf, PaletteEffect* paletteEffects = NULL, bool effectsStopped = false);

		void       clearScreen           (int index);

};


// Variables

EXTERN SDL_Surface* canvas; ///< Surface used for drawing
EXTERN int          canvasW; ///< Drawing surface width
EXTERN int          canvasH; ///< Drawing surface height

EXTERN Video video; ///< Video output


// Functions

EXTERN SDL_Surface*   createSurface     (unsigned char* pixels, int width, int height);
EXTERN void           drawRect          (int x, int y, int width, int height, int index);
EXTERN void           enableColorKey    (SDL_Surface* surface, unsigned int index);
EXTERN unsigned int   getColorKey       (SDL_Surface* surface);
EXTERN void           setLogicalPalette (SDL_Surface* surface, SDL_Color *palette, int start, int length);

#endif


