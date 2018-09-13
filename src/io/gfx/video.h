
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _VIDEO_H
#define _VIDEO_H


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
		SDL_Surface* screen; ///< Output surface

		// Palettes
		SDL_Color*   currentPalette; ///< Current palette
		SDL_Color    logicalPalette[256]; ///< Logical palette (greyscale)
		bool         fakePalette; ///< Whether or not the palette mode is being emulated

		int          maxW; ///< Largest possible width
		int          maxH; ///< Largest possible height
		int          screenW; ///< Real width
		int          screenH; ///< Real height
#ifdef SCALE
		int          scaleFactor; ///< Scaling factor
#endif
		bool         fullscreen; ///< Full-screen mode

		void findMaxResolution ();
		void expose            ();

	public:
		Video ();

		bool       init                  (int width, int height, bool startFullscreen);

		bool       reset                (int width, int height);

		void       setPalette            (SDL_Color *palette);
		SDL_Color* getPalette            ();
		void       changePalette         (SDL_Color *palette, unsigned char first, unsigned int amount);
		void       restoreSurfacePalette (SDL_Surface *surface);

		int        getMaxWidth           ();
		int        getMaxHeight          ();
		int        getWidth              ();
		int        getHeight             ();
#ifdef SCALE
		int        getScaleFactor        ();
		int        setScaleFactor        (int newScaleFactor);
#endif
#ifndef FULLSCREEN_ONLY
		bool       isFullscreen          ();
#endif

		void       update                (SDL_Event *event);
		void       flip                  (int mspf, PaletteEffect* paletteEffects);

		void       clearScreen           (int index);

};


// Variables

EXTERN SDL_Surface* canvas; ///< Surface used for drawing
EXTERN int          canvasW; ///< Drawing surface width
EXTERN int          canvasH; ///< Drawing surface height

EXTERN Video video; ///< Video output


// Functions

EXTERN SDL_Surface*   createSurface  (unsigned char* pixels, int width, int height);
EXTERN void           drawRect       (int x, int y, int width, int height, int index);

#endif


