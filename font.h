
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


// Class

class Font {

	private:
		SDL_Surface   *surface;
		unsigned char *w;
		unsigned char  h; // Dimensions of the letters
		char           map[128]; // Maps ASCII values to letter positions

	public:
		Font                (char *fn);
		Font                (File *f, bool big);
		~Font               ();
		int  showString     (char *s, int x, int y);
		void showNumber     (int n, int x, int y);
		void scalePalette   (fixed scale, signed int offset);
		void restorePalette ();

};


