
/*
 *
 * file.h
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


#ifndef _FILE_H
#define _FILE_H


#include "OpenJazz.h"


// Class

class File {

	private:
		FILE *f;

	public:
		File                           (char * fileName, bool write);
		~File                          ();

		int                getSize     ();
		void               seek        (int offset, bool reset);
		int                tell        ();
		unsigned char      loadChar    ();
		void               storeChar   (unsigned char val);
		unsigned short int loadShort   ();
		void               storeShort  (unsigned short int val);
		signed long int    loadInt     ();
		void               storeInt    (signed long int val);
		unsigned char *    loadBlock   (int length);
		unsigned char *    loadRLE     (int length);
		void               skipRLE     ();
		char *             loadString  ();
		SDL_Surface *      loadSurface (int width, int height);
		void               loadPalette (SDL_Color *palette);

};


//Variable

// Path to game data
Extern char *path;

#endif

