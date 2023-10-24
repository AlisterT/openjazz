
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
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _FONT_H
#define _FONT_H


#include "OpenJazz.h"

#include <SDL.h>


// Classes

class File;

/// Font
class Font {

	private:
		SDL_Surface   *characters[128]; ///< Symbol images
		int            nCharacters; ///< Number of symbols
		unsigned char  lineHeight; ///< Vertical spacing of displayed characters
		char           map[128]; ///< Maps ASCII values to symbol indices

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
		int  getStringWidth      (const char *string);
		int  getSceneStringWidth (const unsigned char *string);

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

