
/**
 *
 * @file file.cpp
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
 * @par Description:
 * Deals with files.
 *
 */


#include "file.h"
#include "file_dir.h"
#include "io/gfx/video.h"
#include "util.h"
#include "io/log.h"

#include <string.h>
#include <unistd.h>
#include <miniz.h>

std::unique_ptr<File> File::open (const char* name, int pathType, bool write) {
	// shortcuts for special paths
	if (pathType & PATH_TYPE_CONFIG) {
		return std::unique_ptr<File>(new DirFile(gamePaths.config, name, write));
	}
	if (pathType & PATH_TYPE_TEMP) {
		return std::unique_ptr<File>(new DirFile(gamePaths.temp, name, write));
	}

	// we do not allow writing anywhere else
	if (write) {
		LOG_FATAL("Not allowed to write to %s", name);

		throw E_FILE;
	}

#ifdef WANT_ZIP

#endif

	Path* path = gamePaths.paths;

	while (path) {
		// skip other paths
		if ((path->pathType & pathType) != pathType) {
			path = path->next;
			continue;
		}

		try {
			return std::unique_ptr<File>(new DirFile(path->path, name, write));
		} catch (int e) {
		}

		path = path->next;
	}

	LOG_WARN("Could not open file: %s", name);

	throw E_FILE;
}

bool File::exists (const char * fileName, int pathType) {
	std::unique_ptr<File> file;

#ifdef VERBOSE
	printf("Check: ");
#endif

	try {
		file = File::open(fileName, pathType);
	} catch (int e) {
		return false;
	}

	return true;
}

unsigned short int File::loadShort () {
	unsigned short int val = loadChar();
	val += loadChar() << 8;

	return val;
}

unsigned short int File::loadShort (unsigned short int max) {
	unsigned short int val = loadShort();
	if (val > max) {
		LOG_ERROR("Oversized value %d>%d in file %s", val, max, filePath);

		return max;
	}

	return val;
}

void File::storeShort (unsigned short int val) {
	storeChar(val & 255);
	storeChar(val >> 8);
}

signed int File::loadInt () {
	unsigned int val = loadChar();
	val += loadChar() << 8;
	val += loadChar() << 16;
	val += loadChar() << 24;

	return *((signed int *)&val);
}

void File::storeInt (signed int val) {
	unsigned int uval = *((unsigned int *)&val);

	storeChar(uval & 255);
	storeChar((uval >> 8) & 255);
	storeChar((uval >> 16) & 255);
	storeChar(uval >> 24);
}

/**
 * Load a block of uncompressed data from the file.
 *
 * @param length The length of the block
 *
 * @return Buffer containing the block of data
 */
unsigned char * File::loadBlock (int length) {
	unsigned char *buffer = new unsigned char[length];

	int res = readBlock(buffer, length);
	if (res != length)
		LOG_ERROR("Could not read whole block (%d of %d bytes read)", res, length);

	return buffer;
}

/**
 * Load a block of RLE compressed data from the file.
 *
 * @param length The length of the uncompressed block
 *
 * @return Buffer containing the uncompressed data
 */
unsigned char* File::loadRLE (int length) {
	// Determine the offset that follows the block
	size_t next = loadChar();
	next += loadChar() << 8;
	next += tell();

	unsigned char* buffer = new unsigned char[length];
	int pos = 0;

	while (pos < length) {
		int rle = loadChar();

		if (rle & 128) {
			int byte = loadChar();

			for (int i = 0; i < (rle & 127); i++) {
				buffer[pos++] = byte;
				if (pos >= length) break;
			}
		} else if (rle) {
			for (int i = 0; i < rle; i++) {
				buffer[pos++] = loadChar();
				if (pos >= length) break;
			}
		} else buffer[pos++] = loadChar();
	}

	seek(next, true);

	return buffer;
}


/**
 * Skip past a block of RLE compressed data in the file.
 */
void File::skipRLE () {
	size_t next = loadChar();
	next += loadChar() << 8;

	seek(next);
}


/**
 * Load a block of LZ compressed data from the file.
 *
 * @param compressedLength The length of the compressed block
 * @param length The length of the uncompressed block
 *
 * @return Buffer containing the uncompressed data
 */
unsigned char* File::loadLZ (int compressedLength, int length) {
	unsigned char* compressedBuffer = loadBlock(compressedLength);
	unsigned char* buffer = new unsigned char[length];

	uncompress(buffer, (unsigned long int *)&length, compressedBuffer, compressedLength);
	delete[] compressedBuffer;

	return buffer;
}

/**
 * Load a string from the file.
 *
 * @return The new string
 */
char * File::loadString () {
	char *string;
	size_t length = loadChar();

	if (length) {
		string = new char[length + 1];
		size_t res = readBlock(string, length);
		if (res != length)
			LOG_ERROR("Could not read whole string (%zu of %zu bytes read)", res, length);
	} else {
		// If the length is not given, assume it is an 8.3 file name
		string = new char[13];
		int i;
		for (i = 0; i < 9; i++) {
			string[i] = loadChar();
			if (string[i] == '.') {
				string[++i] = loadChar();
				string[++i] = loadChar();
				string[++i] = loadChar();
				i++;

				break;
			}
		}
		length = i;
	}
	string[length] = '\0';

	return string;
}

/**
 * Load RLE compressed graphical data from the file.
 *
 * @param width The width of the image to load
 * @param height The height of the image to load
 *
 * @return SDL surface containing the loaded image
 */
// TODO: maybe move to graphics class?
SDL_Surface* File::loadSurface (int width, int height) {
	unsigned char* pixels = loadRLE(width * height);
	SDL_Surface* surface = createSurface(pixels, width, height);
	delete[] pixels;

	return surface;
}

/**
 * Load a block of scrambled pixel data from the file.
 *
 * @param length The length of the block
 *
 * @return Buffer containing the de-scrambled data
 */
unsigned char* File::loadPixels  (int length) {
	unsigned char* sorted = new unsigned char[length];
	unsigned char* pixels = loadBlock(length);

	// Rearrange pixels in correct order
	for (int i = 0; i < length; i++) {
		sorted[i] = pixels[(i >> 2) + ((i & 3) * (length >> 2))];
	}
	delete[] pixels;

	return sorted;
}

/**
 * Load a block of scrambled and masked pixel data from the file.
 *
 * @param length The length of the block
 * @param key The transparent pixel value
 *
 * @return Buffer containing the de-scrambled data
 */
unsigned char* File::loadPixels (int length, int key) {
	unsigned char mask = 0;
	unsigned char* sorted = new unsigned char[length];
	unsigned char* pixels = new unsigned char[length];

	// Read the mask
	// Each mask pixel is either 0 or 1
	// Four pixels are packed into the lower end of each byte
	for (int i = 0; i < length; i++) {
		if (!(i & 3))
			mask = loadChar();
		pixels[i] = (mask >> (i & 3)) & 1;
	}

	// Pixels are loaded if the corresponding mask pixel is 1, otherwise
	// the transparent index is used. Pixels are scrambled, so the mask
	// has to be scrambled the same way.
	for (int i = 0; i < length; i++) {
		sorted[(i >> 2) + ((i & 3) * (length >> 2))] = pixels[i];
	}

	// Read pixels according to the scrambled mask
	for (int i = 0; i < length; i++) {

		// Use the transparent pixel
		pixels[i] = key;
		if (sorted[i] == 1) {
			// The unmasked portions are transparent, so no masked
			// portion should be transparent.
			while (pixels[i] == key)
				pixels[i] = loadChar();
		}
	}

	// Rearrange pixels in correct order
	for (int i = 0; i < length; i++) {
		sorted[i] = pixels[(i >> 2) + ((i & 3) * (length >> 2))];
	}
	delete[] pixels;

	return sorted;
}

/**
 * Load a palette from the file.
 *
 * @param palette The palette to be filled with loaded colours
 * @param rle Whether or not the palette data is RLE-encoded
 */
//TODO: maybe return palette instead
void File::loadPalette (SDL_Color* palette, bool rle) {
	unsigned char* buffer;

	if (rle) buffer = loadRLE(MAX_PALETTE_COLORS * 3);
	else buffer = loadBlock(MAX_PALETTE_COLORS * 3);

	for (int i = 0; i < MAX_PALETTE_COLORS; i++) {
		// Palette entries are 6-bit
		// Shift them upwards to 8-bit, and fill in the lower 2 bits
		palette[i].r = (buffer[i * 3] << 2) + (buffer[i * 3] >> 4);
		palette[i].g = (buffer[(i * 3) + 1] << 2) + (buffer[(i * 3) + 1] >> 4);
		palette[i].b = (buffer[(i * 3) + 2] << 2) + (buffer[(i * 3) + 2] >> 4);
	}

	delete[] buffer;
}


PathMgr::PathMgr():
	paths(NULL), config(nullptr), temp(nullptr) {

};

PathMgr::~PathMgr() {
	if(config)
		delete[] config;
	if(temp)
		delete[] temp;
	delete paths;
};

bool PathMgr::add(char* newPath, int newPathType) {

	// Check for CWD
	if(!strlen(newPath)) {
		delete[] newPath;

		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			newPath = createString(cwd);
		} else {
			LOG_WARN("Could not get current working directory!");
			return false;
		}
	}

	// Append a directory separator if necessary
	if (newPath[strlen(newPath) - 1] != OJ_DIR_SEP) {
		char* tmp = createString(newPath, OJ_DIR_SEP_STR);
		delete[] newPath;
		newPath = tmp;
	}

	// all paths need to be readable
	if (access(newPath, R_OK) != 0) {
		LOG_TRACE("Path '%s' is not readable, ignoring!", newPath);
		delete[] newPath;
		return false;
	}

	// ignore, if already present
	if(config) newPathType &= ~PATH_TYPE_CONFIG;
	if(temp) newPathType &= ~PATH_TYPE_TEMP;

	// config and temp dir need to be writeable
	if ((newPathType & (PATH_TYPE_CONFIG|PATH_TYPE_TEMP)) > 0) {
		if (access(newPath, W_OK) != 0) {
			LOG_WARN("Path '%s' is not writeable, disabling!", newPath);

			newPathType &= ~PATH_TYPE_CONFIG;
			newPathType &= ~PATH_TYPE_TEMP;
		}
	}

	// we only need one directory for these
	if (newPathType & PATH_TYPE_CONFIG) {
		config = createString(newPath);
		newPathType &= ~PATH_TYPE_CONFIG;

		LOG_DEBUG("Using directory '%s' for configuration.", config);
	}
	if (newPathType & PATH_TYPE_TEMP) {
		temp = createString(newPath);
		newPathType &= ~PATH_TYPE_TEMP;

		LOG_DEBUG("Using directory '%s' for temporary files.", temp);
	}

	// path could have been invalidated above
	if(newPathType == PATH_TYPE_INVALID) {
		delete[] newPath;
		return false;
	}

	// Finally add
	paths = new Path(paths, newPath, newPathType);

	return true;
}


/**
 * Create a new directory path object.
 *
 * @param newNext Next path
 * @param newPath The new path
 */
Path::Path (Path* newNext, char* newPath, int newPathType) {

	char pathInfo[11] = {};
	if(newPathType & PATH_TYPE_SYSTEM) strcat(pathInfo, "Sys");
	if(newPathType & PATH_TYPE_USER) strcat(pathInfo, "Usr");
	if(newPathType & PATH_TYPE_GAME) strcat(pathInfo, "Game");

	LOG_DEBUG("Adding '%s' to the path list [%s]", newPath, pathInfo);

	next = newNext;
	path = newPath;
	pathType = newPathType;

}


/**
 * Delete the directory path object.
 */
Path::~Path () {

	if (next) delete next;
	delete[] path;

}
