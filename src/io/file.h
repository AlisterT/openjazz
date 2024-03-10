
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
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _FILE_H
#define _FILE_H

#include "OpenJazz.h"

#include <SDL.h>
#include <memory>
#include <cstdio>
#include <vector>
#include <string>

// Classes

/// File i/o
class File {

	private:
		FILE* file;
		char* filePath;
		bool  forWriting;

		bool open (std::string path, const char* name, bool write);

	public:
		File                           (const char* name, int pathType, bool write = false);
		~File                          ();

		int                getSize     ();
		void               seek        (int offset, bool reset = false);
		int                tell        ();
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
#ifdef ENABLE_JJ2
		unsigned char*     loadLZ      (int compressedLength, unsigned int length);
#endif
		char*              loadString  ();
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
	PATH_TYPE_SYSTEM = 1 << 0, ///< System directory
	PATH_TYPE_USER = 1 << 1,   ///< User directory
	PATH_TYPE_CONFIG = 1 << 2, ///< User writable configuration directory
	PATH_TYPE_GAME = 1 << 3,   ///< Directory containing game data
	PATH_TYPE_TEMP = 1 << 4    ///< User writable temporary directory
};

class Path {

	public:
		Path(const char* newPath, int newPathType);
		Path(std::string const &newPath, int newPathType);

		std::string path;   ///< Path
		int pathType;       ///< One or more of path_type enum

};


class PathMgr {

	public:
		PathMgr() = default;

		bool add(char* newPath, int newPathType);

		const char *getConfig() const { return config.c_str(); };
		const char *getTemp() const { return temp.c_str(); };
		std::vector<Path> getPaths(int pathType) const;

	private:
		std::vector<Path> paths;
		std::string config;
		std::string temp;

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

