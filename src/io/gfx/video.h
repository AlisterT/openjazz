
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

#ifndef _GRAPHICS_H
#define _GRAPHICS_H


#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

#define V_WINDOWED (SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)

#if defined(WIZ) || defined(GP2X)
	#define V_FULLSCREEN (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#else
	#define V_FULLSCREEN (SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE)
#endif

#ifdef SCALE
	#define MIN_SCALE 1
	#define MAX_SCALE 4
#endif

// Black palette index
#define BLACK 31


// Variables

EXTERN SDL_Surface *screen, *canvas;
EXTERN int          viewW, viewH, canvasW, canvasH, screenW, screenH;
#ifdef SCALE
EXTERN int          scaleFactor;
#endif
#ifndef FULLSCREEN_ONLY
EXTERN bool         fullscreen;
#endif
EXTERN bool         fakePalette;

// Palettes
EXTERN SDL_Color *currentPalette;
EXTERN SDL_Color  logicalPalette[256];

// Panel
EXTERN SDL_Surface *panel;
EXTERN SDL_Surface *panelAmmo[5];


// Functions

EXTERN SDL_Surface * createSurface    (unsigned char *pixels, int width, int height);
EXTERN void          createScreen     ();
EXTERN void          usePalette       (SDL_Color *palette);
EXTERN void          restorePalette   (SDL_Surface *surface);
EXTERN void          clearScreen      (int index);
EXTERN void          drawRect         (int x, int y, int width, int height, int index);

#endif


