
/*
 *
 * font.h
 *
 * Created on the 3rd of February 2009 from parts of OpenJazz.h
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


#include "file.h"


// Class

class Font {

	private:
		SDL_Surface   *surface;
		unsigned char *w;
		unsigned char  h; // Dimensions of the letters
		char           map[128]; // Maps ASCII values to letter positions

	public:
		Font                (char *fileName);
		Font                (File *file, bool big);
		~Font               ();

		int  showString     (char *s, int x, int y);
		void showNumber     (int n, int x, int y);
		void mapPalette     (int start, int length, int newStart,
			int newLength);
		void restorePalette ();

};

// Variables

Extern Font *font2;          /* Taken from .0FN file name */
Extern Font *fontbig;        /* Taken from .0FN file name */
Extern Font *fontiny;        /* Taken from .0FN file name */
Extern Font *fontmn1;        /* Taken from .0FN file name */
Extern Font *fontmn2;        /* Taken from .0FN file name */
Extern Font *panelBigFont;   /* Not a font file, found in PANEL.000 */
Extern Font *panelSmallFont; /* Not a font file, found in PANEL.000 */

#endif

