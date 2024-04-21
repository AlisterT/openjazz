
/**
 *
 * @file file.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#ifndef OJ_FILE_H
#define OJ_FILE_H

#include <memory>
#include "OpenJazz.h"
#include <SDL.h>

// Classes

/// File i/o
class File {
	public:
		/**
		 * Delete the file object.
		 */
		virtual ~File () {};

		/**
		 * Try opening a file from the available paths.
		 *
		 * @param name File name
		 * @param pathType Kind of directory
		 * @param write Whether or not the file can be written to
		 */
		static std::unique_ptr<File> open(const char* name, int pathType, bool write = false);

		/**
		 * Check if a file exists.
		 *
		 * @param fileName The file to check
		 * @param pathType Kind of directory
		 *
		 * @return Whether or not the file exists
		 */
		static bool exists(const char* name, int pathType);

		/**
		 * Returns the size of the file.
		 */
		virtual size_t getSize () =0;

		/**
		 * Set the read/write location within the file.
		 *
		 * @param offset The new offset
		 * @param reset Whether to offset from the current location or the start of the file
		 */
		virtual void seek (size_t offset, bool reset = false) =0;

		/**
		 * Returns the current read/write location within the file.
		 */
		virtual size_t tell () =0;

		/**
		 * Returns an unsigned char from the file.
		 */
		virtual unsigned char loadChar () =0;

		virtual void storeChar (unsigned char val) =0;

		/**
		 * Returns an unsigned short int from the file.
		 */
		virtual unsigned short int loadShort  ();

		/**
		 * Returns an unsigned short int with an upper limit from the file.
		 */
		virtual unsigned short int loadShort  (unsigned short int max);

		virtual void               storeShort (unsigned short int val);

		/**
		 * Returns a signed int from the file.
		 */
		virtual signed int         loadInt    ();

		virtual void               storeInt   (signed int val);

		unsigned char* loadBlock   (int length);
		unsigned char* loadRLE     (int length);
		void           skipRLE     ();
		unsigned char* loadLZ      (int compressedLength, int length);
		char*          loadString  ();
		SDL_Surface*   loadSurface (int width, int height);
		unsigned char* loadPixels  (int length);
		unsigned char* loadPixels  (int length, int key);
		void           loadPalette (SDL_Color* palette, bool rle = true);

	protected:
		File() {};
		char* filePath;

	private:
		virtual size_t readBlock (void* buffer, size_t length) =0;
};


/// Directory path

enum path_type {
	PATH_TYPE_INVALID = 0,     ///< Invalid directory, do not use
	PATH_TYPE_SYSTEM = 1 << 0, ///< Read-only system directory
	PATH_TYPE_CONFIG = 1 << 1, ///< User writable configuration directory
	PATH_TYPE_GAME = 1 << 2,   ///< Directory containing game data
	PATH_TYPE_TEMP = 1 << 3,   ///< User writable temporary directory
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

		bool add(char* newPath, int newPathType);

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

