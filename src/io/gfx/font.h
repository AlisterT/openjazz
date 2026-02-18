
/**
 *
 * @file font.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 3rd February 2009: Created font.h from parts of OpenJazz.h
 *
 * @par Licence:
 * Copyright (c) 2005-2010 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#ifndef OJ_FONT_H
#define OJ_FONT_H

#include "OpenJazz.h"

#ifdef OJ_SDL3
	#include <SDL3/SDL.h>
#else
	#include <SDL.h>
#endif

#define MAX_FONT_CHARS 128

// Classes

/// Font
class Font {

	private:
		void           commonSetup();
		void           cleanMapping();
		SDL_Surface   *characterAtlas; ///< Symbol images
		SDL_Rect       atlasRects[MAX_FONT_CHARS]; ///< Symbol positions
		bool           isOk; ///< Font is loaded and usable
		int            nCharacters; ///< Number of symbols
		unsigned char  spaceWidth; ///< Horizontal spacing of displayed characters
		unsigned char  lineHeight; ///< Vertical spacing of displayed characters
		unsigned int   map[MAX_FONT_CHARS]; ///< Maps ASCII values to symbol indices

	public:
		explicit Font(const char *fileName);
		Font(unsigned char *pixels, bool big);
		explicit Font(bool bonus);
		~Font();

		int  showString          (const char *s, int x, int y);
		int  showSceneString     (const unsigned char *s, int x, int y);
		void showNumber          (int n, int x, int y);
		void mapPalette          (int start, int length, int newStart, int newLength);
		void restorePalette      ();
		int  getHeight           ();
		int  getSpaceWidth       ();
		int  getStringWidth      (const char *string);
		int  getSceneStringWidth (const unsigned char *string);
#ifdef DEBUG_FONTS
		void saveAtlasAsBMP      (const char *fileName);
#endif

};


// Variables

EXTERN Font *font2;          /** Taken from .0FN file name */
EXTERN Font *fontbig;        /** Taken from .0FN file name */
EXTERN Font *fontiny;        /** Taken from .0FN file name */
EXTERN Font *fontmn1;        /** Taken from .0FN file name */
EXTERN Font *fontmn2;        /** Taken from .0FN file name */
EXTERN Font *panelBigFont;   /** Found in PANEL.000 */
EXTERN Font *panelSmallFont; /** Found in PANEL.000 */

#endif
