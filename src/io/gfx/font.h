
/*
 *
 * font.h
 *
 * 3rd February 2009: Created font.h from parts of OpenJazz.h
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _FONT_H
#define _FONT_H


#include "OpenJazz.h"

#include <SDL/SDL.h>


// Classes

class File;

class Font {

	private:
		SDL_Surface   *surface;
		unsigned char *w;
		unsigned char  h; // Dimensions of the letters
		char           map[128]; // Maps ASCII values to letter positions

	public:
		Font                     (const char *fileName);
		Font                     (File *file, bool big);
		~Font                    ();

		int  showString          (const char *s, int x, int y);
		int  showSceneString     (const char *s, int x, int y);
		void showNumber          (int n, int x, int y);
		void mapPalette          (int start, int length, int newStart, int newLength);
		void restorePalette      ();
		int  getHeight           ();
		int  getStringWidth      (const char *string);
		int  getSceneStringWidth (const char *string);
};

// Variables

EXTERN Font *font2;          /* Taken from .0FN file name */
EXTERN Font *fontbig;        /* Taken from .0FN file name */
EXTERN Font *fontiny;        /* Taken from .0FN file name */
EXTERN Font *fontmn1;        /* Taken from .0FN file name */
EXTERN Font *fontmn2;        /* Taken from .0FN file name */
EXTERN Font *panelBigFont;   /* Not a font file, found in PANEL.000 */
EXTERN Font *panelSmallFont; /* Not a font file, found in PANEL.000 */

#endif

