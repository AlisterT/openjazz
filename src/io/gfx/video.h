
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
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#ifndef _VIDEO_H
#define _VIDEO_H

#include "setup.h"
#include "paletteeffects.h"
#include "platforms/platforms.h"

#ifdef OJ_SDL3
	#include <SDL3/SDL.h>
#else
	#include <SDL.h>
#endif

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

// Default configuration
#ifndef DEFAULT_SCREEN_WIDTH
	#define DEFAULT_SCREEN_WIDTH SW
#endif
#ifndef DEFAULT_SCREEN_HEIGHT
	#define DEFAULT_SCREEN_HEIGHT SH
#endif

// Fullscreen and Window flags are only for SDL1.2
#if OJ_SDL2 || OJ_SDL3
	#ifdef WINDOWED_FLAGS
		#pragma message "Ignoring WINDOWED_FLAGS when not building with SDL 1.2"
	#endif
	#ifdef FULLSCREEN_FLAGS
		#pragma message "Ignoring FULLSCREEN_FLAGS when not building with SDL 1.2"
	#endif
#else
	#ifndef WINDOWED_FLAGS
		#define WINDOWED_FLAGS (SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)
	#endif
	#ifndef FULLSCREEN_FLAGS
		#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)
	#endif
#endif

// Time interval
#define T_MENU_FRAME 20

// Class

/// Video output
class Video {
	public:
		Video ();

		bool       init                  (SetupOptions cfg);
		void       deinit                ();
		bool       reset                 (int width, int height);

		void       setPalette            (SDL_Color *palette);
		SDL_Color* getPalette            () const;
		void       changePalette         (SDL_Color *palette, unsigned char first, unsigned int amount);

		// Surface
		SDL_Surface* createSurface       (const unsigned char* pixels, int width, int height);
		void         destroySurface      (SDL_Surface *surface);
		void         setSurfacePalette   (SDL_Surface *surface, SDL_Color *palette, int start, int length);
		void         restoreSurfacePalette (SDL_Surface *surface);
		void         enableColorKey      (SDL_Surface *surface, unsigned int index);
		unsigned int getColorKey         (SDL_Surface *surface);
		void         setClipRect         (SDL_Surface *surface, const SDL_Rect *rect);

		void       drawRect              (int x, int y, int width, int height, int index, bool fill = true);

		int        getMinWidth           () const;
		int        getMaxWidth           () const;
		int        getMinHeight          () const;
		int        getMaxHeight          () const;
		int        getWidth              () const;
		int        getHeight             () const;
		void       setTitle              (const char *title);
		int        getScaleFactor        () const;
		scalerType getScaleMethod        () const;
		void       setScaling            (int newScaleFactor, scalerType newScaleMethod);
		bool       isFullscreen          () const;

		void       moviePlayback         (bool status);

		void       update                (SDL_Event *event);
		void       flip                  (int mspf, PaletteEffect* paletteEffects = NULL, bool effectsStopped = false);

		void       clearScreen           (int index);

	private:
		void       findResolutions ();
		void       expose          ();
		void       commonDeinit    ();

#if OJ_SDL3 || OJ_SDL2
		SDL_Window*   window; ///< Output window
		SDL_Renderer* renderer; ///< Output renderer
		SDL_Texture*  texture; ///< Output texture
#endif
#if OJ_SDL2
		SDL_Surface*  textureSurface;
#endif
		SDL_Surface*  screen; ///< Output surface

		// Palettes
		SDL_Color*    currentPalette; ///< Current palette
#if OJ_SDL3
		SDL_Palette*  texturePalette; ///< Screen palette
		SDL_Palette*  logicalPalette; ///< Logical palette (greyscale)
#else
		SDL_Color     logicalPalette[MAX_PALETTE_COLORS]; ///< Logical palette (greyscale)
#endif

		int           minW; ///< Smallest possible width
		int           maxW; ///< Largest possible width
		int           minH; ///< Smallest possible height
		int           maxH; ///< Largest possible height
		int           screenW; ///< Real width
		int           screenH; ///< Real height
		int           scaleFactor; ///< Scaling factor
		scalerType    scaleMethod; ///< Filtering
		bool          fullscreen; ///< Full-screen mode
		bool          isPlayingMovie;
};

// Inline functions

inline SDL_Color* Video::getPalette () const { return currentPalette; } ///< Returns the current display palette.
inline int Video::getMinWidth () const { return minW; } ///< Returns the minimum possible screen width.
inline int Video::getMaxWidth () const { return maxW; } ///< Returns the maximum possible screen width.
inline int Video::getMinHeight () const { return minH; } ///< Returns the minimum possible screen height.
inline int Video::getMaxHeight () const { return maxH; } ///< Returns the maximum possible screen height.
inline int Video::getWidth () const { return screenW; } ///< Returns the current width of the window or screen.
inline int Video::getHeight () const { return screenH; } ///< Returns the current height of the window or screen.
inline int Video::getScaleFactor () const { return scaleFactor;} ///< Returns the current scaling factor.
inline scalerType Video::getScaleMethod () const { return scaleMethod; } ///< Returns the current scaling method.
inline bool Video::isFullscreen () const { return fullscreen; } ///< Determines whether or not full-screen mode is being used.

// Variables

EXTERN SDL_Surface* canvas; ///< Surface used for drawing
EXTERN int          canvasW; ///< Drawing surface width
EXTERN int          canvasH; ///< Drawing surface height

EXTERN Video video; ///< Video output

#endif
