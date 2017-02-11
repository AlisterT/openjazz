
/**
 *
 * @file file.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 3rd February 2009: Created file.h from parts of OpenJazz.h
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


#ifndef _FILE_H
#define _FILE_H


#include "OpenJazz.h"

#include <SDL/SDL.h>
#include <stdio.h>


// Classes

/// File i/o
class File {

	private:
		FILE* file;
		char* filePath;

		bool open (const char* path, const char* name, bool write);

	public:
		File                           (const char* name, bool write);
		~File                          ();

		int                getSize     ();
		void               seek        (int offset, bool reset);
		int                tell        ();
		unsigned char      loadChar    ();
		void               storeChar   (unsigned char val);
		unsigned short int loadShort   ();
		unsigned short int loadShort   (unsigned short int max);
		void               storeShort  (unsigned short int val);
		signed long int    loadInt     ();
		void               storeInt    (signed long int val);
		unsigned char*     loadBlock   (int length);
		unsigned char*     loadRLE     (int length);
		void               skipRLE     ();
		unsigned char*     loadLZ      (int compressedLength, int length);
		char*              loadString  ();
		SDL_Surface*       loadSurface (int width, int height);
		unsigned char*     loadPixels  (int length);
		unsigned char*     loadPixels  (int length, int key);
		void               loadPalette (SDL_Color* palette, bool rle = true);

};

/// Directory path
class Path {

	public:
		Path* next; ///< Next path to check
		char* path; ///< Path

		Path  (Path* newNext, char* newPath);
		~Path ();

};


// Variable

// Paths to files
EXTERN Path* firstPath;

#endif

