
/*
 *
 * video.h
 *
 * 13th July 2009: Created graphics.h from parts of OpenJazz.h
 * 26th July 2009: Renamed graphics.h to video.h
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

#ifndef _VIDEO_H
#define _VIDEO_H


#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Original screen dimensions
#define SW 320
#define SH 200

#define WINDOWED_FLAGS (SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)

#if defined(WIZ) || defined(GP2X)
	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#else
	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)
#endif

#ifdef SCALE
	#define MIN_SCALE 1
	#define MAX_SCALE 4
#endif


// Class

class Video {

	private:
		SDL_Surface* screen;

		// Palettes
		SDL_Color*   currentPalette;
		SDL_Color    logicalPalette[256];
		bool         fakePalette;

		int          screenW, screenH;
#ifdef SCALE
		int          scaleFactor;
#endif
#ifndef FULLSCREEN_ONLY
		bool         fullscreen;
#endif

	public:
		Video  ();

		bool       create                (int width, int height);

		void       setPalette            (SDL_Color *palette);
		SDL_Color* getPalette            ();
		void       changePalette         (SDL_Color *palette, unsigned char first, unsigned char amount);
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
		void       flip                  (int mspf);

};


// Variables

EXTERN SDL_Surface* canvas;
EXTERN int          viewH, canvasW, canvasH;
#define viewW canvasW

// Panel
EXTERN SDL_Surface* panel;
EXTERN SDL_Surface* panelAmmo[5];

EXTERN Video video;


// Functions

EXTERN SDL_Surface*   createSurface  (unsigned char* pixels, int width, int height);
EXTERN void           clearScreen    (int index);
EXTERN void           drawRect       (int x, int y, int width, int height, int index);

#endif


