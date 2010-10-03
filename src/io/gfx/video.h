
/**
 *
 * @file video.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 13th July 2009: Created graphics.h from parts of OpenJazz.h
 * 26th July 2009: Renamed graphics.h to video.h
 *
 * @section Licence
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

#ifndef _VIDEO_H
#define _VIDEO_H


#include "paletteeffects.h"

#include <SDL/SDL.h>


// Constants

// Original screen dimensions
#define SW 320
#define SH 200

#define WINDOWED_FLAGS (SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#elif defined(DINGOO)
	#define FULLSCREEN_FLAGS 0
#else
	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)
#endif

#ifdef SCALE
	#define MIN_SCALE 1
	#define MAX_SCALE 4
#endif


// Class

/// Video output
class Video {

	private:
		SDL_Surface* screen; ///< Output surface

		// Palettes
		SDL_Color*   currentPalette; ///< Current palette
		SDL_Color    logicalPalette[256]; ///< Logical palette (greyscale)
		bool         fakePalette; ///< Whether or not the palette mode is being emulated

		int          screenW; ///< Real width
		int          screenH; ///< Real height
#ifdef SCALE
		int          scaleFactor; ///< Scaling factor
#endif
#ifndef FULLSCREEN_ONLY
		bool         fullscreen; ///< Full-screen mode
#endif

	public:
		Video ();

		bool       create                (int width, int height);

		void       setPalette            (SDL_Color *palette);
		SDL_Color* getPalette            ();
		void       changePalette         (SDL_Color *palette, unsigned char first, unsigned int amount);
		void       restoreSurfacePalette (SDL_Surface *surface);

		int        getWidth              ();
		int        getHeight             ();
#ifdef SCALE
		int        getScaleFactor        ();
		void       setScaleFactor        (int newScaleFactor);
#endif
#ifndef FULLSCREEN_ONLY
		bool       isFullscreen          ();
		void       flipFullscreen        ();
#endif

		void       expose                ();
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


