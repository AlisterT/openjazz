
/**
 *
 * @file file.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 3rd February 2009: Created file.h from parts of OpenJazz.h
 *
 * @par Licence:
 * Copyright (c) 2005-2010 AJ Thomson
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _FILE_H
#define _FILE_H


#include "OpenJazz.h"
#include "io/gfx/video.h"

#include <stdio.h>
#include <memory>

// Classes

struct SDL_Surface;
struct SDL_Color;

/// File i/o
class File {

	private:
		FILE* file;
		char* filePath;
		bool  forWriting;

		bool open (const char* path, const char* name, bool write);

	public:
		File                           (const char* name, int pathType, bool write = false);
		~File                          ();

		int                getSize     ();
		void               seek        (int offset, bool reset);
		int                seek        (int offset, int origin = SEEK_CUR);
		int                tell        ();
		int                read        (void *buffer, int size, int count);
		unsigned char      loadChar    ();
		void               storeChar   (unsigned char val);
		unsigned short int loadShort   ();
		unsigned short int loadShort   (unsigned short int max);
		void               storeShort  (unsigned short int val);
		signed int         loadInt     ();
		void               storeInt    (signed int val);
		void               storeData   (void* data, int length);
		unsigned char*     loadBlock   (int length);
		unsigned char*     loadRLE     (int length, bool checkSize = true);
		void               skipRLE     ();
		unsigned char*     loadLZ      (int compressedLength, int length);
		char*              loadTerminatedString (int maxSize = 0);
		char*              loadFileName();
		char*              loadString  (int length);
		SDL_Surface*       loadSurface (int width, int height, bool checkSize = true);
		unsigned char*     loadPixels  (int length);
		unsigned char*     loadPixels  (int length, int key);
		void               loadPalette (SDL_Color* palette, bool rle = true);

};

using FilePtr = std::unique_ptr<File>;


/// Directory path

enum path_type {
	PATH_TYPE_INVALID = 0,     ///< Invalid directory, do not use
	PATH_TYPE_SYSTEM = 1 << 0, ///< Read-only system directory
	PATH_TYPE_CONFIG = 1 << 1, ///< User writable configuration directory
	PATH_TYPE_GAME = 1 << 2,   ///< Directory containing game data
	PATH_TYPE_TEMP = 1 << 3,   ///< User writable temporary directory
	PATH_TYPE_ANY =  1 << 4    ///< Special case: any type
};

class Path {

	public:
		Path* next;      ///< Next path to check
		char* path;      ///< Path
		int   pathType;  ///< One or more of path_type enum

		Path  (Path* newNext, char* newPath, int newPathType);
		~Path ();

};


class PathMgr {

	public:
		PathMgr();
		~PathMgr();

		bool add(char* newPath, int newPathType = PATH_TYPE_ANY);

		Path* paths;

		bool has_config;
		bool has_temp;

};


// Directory Seperator

#ifdef _WIN32
	#define OJ_DIR_SEP '\\'
	#define OJ_DIR_SEP_STR "\\"
#else
	#define OJ_DIR_SEP '/'
	#define OJ_DIR_SEP_STR "/"
#endif


// Variable

EXTERN PathMgr gamePaths; ///< Paths to files

#endif

