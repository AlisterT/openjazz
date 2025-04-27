
/**
 *
 * @file file.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed util.c to util.cpp
 * - 3rd February 2009: Created file.cpp from parts of util.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with files.
 *
 */


#include "file.h"

#include "io/gfx/video.h"
#include "util.h"
#include "io/log.h"

#include <string.h>
#include <unistd.h>
#include <miniz.h>

#if !(defined(_WIN32) || defined(WII) || defined(PSP))
    #define UPPERCASE_FILENAMES
    #define LOWERCASE_FILENAMES
#endif


/**
 * Try opening a file from the available paths.
 *
 * @param name File name
 * @param pathType Kind of directory
 * @param write Whether or not the file can be written to
 */
File::File (const char* name, int pathType, bool write) :
	file(nullptr), filePath(nullptr), forWriting(write) {

	Path* path = gamePaths.paths;

	while (path) {

		// skip other paths
		if (pathType != PATH_TYPE_ANY && (path->pathType & pathType) != pathType) {
			path = path->next;
			continue;
		}

		// only allow certain write paths
		if (!write || (pathType & (PATH_TYPE_CONFIG|PATH_TYPE_TEMP)) > 0) {
			if (open(path->path, name, write)) return;
		} else LOG_FATAL("Not allowed to write to %s", name);

		path = path->next;

	}

	LOG_WARN("Could not open file: %s", name);

	throw E_FILE;

}


/**
 * Delete the file object.
 */
File::~File () {

	fclose(file);

	LOG_TRACE("Closed file: %s", filePath);

	delete[] filePath;

}


/**
 * Try opening a file from the given path
 *
 * @param path Directory path
 * @param name File name
 * @param write Whether or not the file can be written to
 */
bool File::open (const char* path, const char* name, bool write) {
	const char *fileMode = write ? "wb": "rb";

	// Create the file path for the given directory
	filePath = createString(path, name);

	// Open the file from the path
	file = fopen(filePath, fileMode);

#ifdef UPPERCASE_FILENAMES
	if (!file) {
		uppercaseString(filePath + strlen(path));
		file = fopen(filePath, fileMode);
	}
#endif

#ifdef LOWERCASE_FILENAMES
	if (!file) {
		lowercaseString(filePath + strlen(path));
		file = fopen(filePath, fileMode);
	}
#endif

	if (file) {
		LOG_DEBUG("Opened file: %s", filePath);

		return true;
	}

	delete[] filePath;

	return false;
}


/**
 * Get the size of the file.
 *
 * @return The size of the file
 */
int File::getSize () {

	int pos, size;

	pos = ftell(file);

	fseek(file, 0, SEEK_END);

	size = ftell(file);

	fseek(file, pos, SEEK_SET);

	return size;

}


/**
 * Get the current read/write location within the file.
 *
 * @return The current location
 */
int File::tell () {

	return ftell(file);

}


/**
 * Set the read/write location within the file.
 *
 * @param offset The new offset
 * @param reset Whether to offset from the current location or the start of the file
 */
void File::seek (int offset, bool reset) {

	fseek(file, offset, reset ? SEEK_SET: SEEK_CUR);

}


/**
 * Load an unsigned char from the file.
 *
 * @return The value read
 */
unsigned char File::loadChar () {

	return fgetc(file);

}


void File::storeChar (unsigned char val) {

	fputc(val, file);

}


/**
 * Load an unsigned short int from the file.
 *
 * @return The value read
 */
unsigned short int File::loadShort () {

	unsigned short int val;

	val = fgetc(file);
	val += fgetc(file) << 8;

	return val;

}


/**
 * Load an unsigned short int with an upper limit from the file.
 *
 * @return The value read
 */
unsigned short int File::loadShort (unsigned short int max) {

	unsigned short int val;

	val = loadShort();

	if (val > max) {

		LOG_ERROR("Oversized value %d>%d in file %s", val, max, filePath);

		return max;

	}

	return val;

}


void File::storeShort (unsigned short int val) {

	fputc(val & 255, file);
	fputc(val >> 8, file);

}


/**
 * Load a signed int from the file.
 *
 * @return The value read
 */
signed int File::loadInt () {

	unsigned int val;

	val = fgetc(file);
	val += fgetc(file) << 8;
	val += fgetc(file) << 16;
	val += fgetc(file) << 24;

	return *((signed int *)&val);

}


void File::storeInt (signed int val) {

	unsigned int uval;

	uval = *((unsigned int *)&val);

	fputc(uval & 255, file);
	fputc((uval >> 8) & 255, file);
	fputc((uval >> 16) & 255, file);
	fputc(uval >> 24, file);

}


void File::storeData (void* data, int length) {

	if(!forWriting) {
		LOG_ERROR("File %s not opened for writing!", filePath);
		return;
	}

	fwrite (data, length, 1, file);

}


/**
 * Load a block of uncompressed data from the file.
 *
 * @param length The length of the block
 *
 * @return Buffer containing the block of data
 */
unsigned char * File::loadBlock (int length) {

	unsigned char *buffer;

	buffer = new unsigned char[length];

	int res = fread(buffer, 1, length, file);

	if (res != length)
		LOG_ERROR("Could not read whole block (%d of %d bytes read)", res, length);

	return buffer;

}


/**
 * Load a block of RLE compressed data from the file.
 *
 * @param length The length of the uncompressed block
 * @param checkSize Whether or not the RLE block is terminated
 *
 * @return Buffer containing the uncompressed data
 */
unsigned char* File::loadRLE (int length, bool checkSize) {
	int size = 0;

	if (checkSize) {
		// Determine the offset that follows the block
		size = loadShort();
	}
	int start = tell();

	unsigned char* buffer = new unsigned char[length];

	int pos = 0;
	while (pos < length) {
		unsigned char code = loadChar();
		unsigned char amount = code & 127;

		if (code & 128) { // repeat
			unsigned char value = loadChar();

			if (pos + amount >= length) break;

			memset(buffer + pos, value, amount);
			pos += amount;
		} else if (amount) { // copy
			if (pos + amount >= length) break;

			fread(buffer + pos, 1, amount, file);
			pos += amount;
		} else { // end marker
			buffer[pos++] = loadChar();
			break;
		}
	}

	if (checkSize) {
		if (tell() != start + size)
			LOG_DEBUG("RLE block has incorrect size: %d vs. %d", tell() - start, size);

		seek(start + size, true);
	} else {
		LOG_MAX("RLE block was %d bytes long", tell() - start);
	}

	return buffer;

}


/**
 * Skip past a block of RLE compressed data in the file.
 */
void File::skipRLE () {

	int next;

	next = fgetc(file);
	next += fgetc(file) << 8;

	fseek(file, next, SEEK_CUR);

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

	unsigned char* compressedBuffer;
	unsigned char* buffer;

	compressedBuffer = loadBlock(compressedLength);

	buffer = new unsigned char[length];

	uncompress(buffer, (unsigned long int *)&length, compressedBuffer, compressedLength);

	delete[] compressedBuffer;

	return buffer;

}


/**
 * Load a terminated string from the file.
 *
 * @param maxSize maximum length of field in the file
 *
 * @return The new string
 */
char * File::loadTerminatedString (int maxSize) {
	char *string;
	int length = loadChar();

	if (maxSize > 0 && length > maxSize) {
		LOG_WARN("Trimming oversized terminated string (%d > %d)", length, maxSize);
		length = maxSize;
	}

	if (length) {
		string = loadString(length);
	} else {
		string = new char[1];
		string[0] = '\0';
	}

	// Skip until end of field
	if(maxSize > length) {
		seek(maxSize - length);
	}

	return string;
}


/**
 * Load a 8.3 file name from the file.
 *
 * @return The new string
 */
char * File::loadFileName () {
	int length = 0;
	char *string = new char[13];

	for (int i = 0; i < 9; i++) {
		string[i] = loadChar();

		if (string[i] == '.') {
			string[++i] = loadChar();
			string[++i] = loadChar();
			string[++i] = loadChar();
			i++;

			break;
		}

		length = i;
	}

	string[length] = 0;

	return string;
}



/**
 * Load a string with given length from the file.
 *
 * @return The new string
 */
char * File::loadString (int length) {
	char *string = new char[length + 1];
	int res = fread(string, 1, length, file);

	if (res != length)
		LOG_ERROR("Could not read whole string (%d of %d bytes read)", res, length);

	string[length] = '\0';

	return string;
}


/**
 * Load RLE compressed graphical data from the file.
 *
 * @param width The width of the image to load
 * @param height The height of the image to load
 * @param checkSize Whether or not the RLE block is terminated
 *
 * @return SDL surface containing the loaded image
 */
SDL_Surface* File::loadSurface (int width, int height, bool checkSize) {

	SDL_Surface* surface;
	unsigned char* pixels;

	pixels = loadRLE(width * height, checkSize);

	surface = createSurface(pixels, width, height);

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

	unsigned char* pixels;
	unsigned char* sorted;
	int count;

	sorted = new unsigned char[length];

	pixels = loadBlock(length);

	// Rearrange pixels in correct order
	for (count = 0; count < length; count++) {

		sorted[count] = pixels[(count >> 2) + ((count & 3) * (length >> 2))];

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

	unsigned char* pixels;
	unsigned char* sorted;
	unsigned char mask = 0;
	int count;


	sorted = new unsigned char[length];
	pixels = new unsigned char[length];


	// Read the mask
	// Each mask pixel is either 0 or 1
	// Four pixels are packed into the lower end of each byte
	for (count = 0; count < length; count++) {

		if (!(count & 3)) mask = fgetc(file);
		pixels[count] = (mask >> (count & 3)) & 1;

	}

	// Pixels are loaded if the corresponding mask pixel is 1, otherwise
	// the transparent index is used. Pixels are scrambled, so the mask
	// has to be scrambled the same way.
	for (count = 0; count < length; count++) {

		sorted[(count >> 2) + ((count & 3) * (length >> 2))] = pixels[count];

	}

	// Read pixels according to the scrambled mask
	for (count = 0; count < length; count++) {

		// Use the transparent pixel
		pixels[count] = key;

		if (sorted[count] == 1) {

			// The unmasked portions are transparent, so no masked
			// portion should be transparent.
			while (pixels[count] == key) pixels[count] = fgetc(file);

		}

	}

	// Rearrange pixels in correct order
	for (count = 0; count < length; count++) {

		sorted[count] = pixels[(count >> 2) + ((count & 3) * (length >> 2))];

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
void File::loadPalette (SDL_Color* palette, bool rle) {

	unsigned char* buffer;
	int count;

	if (rle) buffer = loadRLE(MAX_PALETTE_COLORS * 3);
	else buffer = loadBlock(MAX_PALETTE_COLORS * 3);

	for (count = 0; count < MAX_PALETTE_COLORS; count++) {

		// Palette entries are 6-bit
		// Shift them upwards to 8-bit, and fill in the lower 2 bits
		palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
		palette[count].g = (buffer[(count * 3) + 1] << 2) + (buffer[(count * 3) + 1] >> 4);
		palette[count].b = (buffer[(count * 3) + 2] << 2) + (buffer[(count * 3) + 2] >> 4);

	}

	delete[] buffer;

}


PathMgr::PathMgr():
	paths(NULL), has_config(false), has_temp(false) {

};

PathMgr::~PathMgr() {
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
	if(has_config) newPathType &= ~PATH_TYPE_CONFIG;
	if(has_temp) newPathType &= ~PATH_TYPE_TEMP;

	// config and temp dir need to be writeable
	if ((newPathType & (PATH_TYPE_CONFIG|PATH_TYPE_TEMP)) > 0) {
		if (access(newPath, W_OK) != 0) {
			LOG_WARN("Path '%s' is not writeable, disabling!", newPath);

			newPathType &= ~PATH_TYPE_CONFIG;
			newPathType &= ~PATH_TYPE_TEMP;
		}
	}

	if(newPathType == PATH_TYPE_INVALID) {
		delete[] newPath;
		return false;
	}

	// we only need one directory for these
	if (newPathType & PATH_TYPE_CONFIG) has_config = true;
	if (newPathType & PATH_TYPE_TEMP) has_temp = true;

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

	char pathInfo[10] = {};
	if(newPathType & PATH_TYPE_SYSTEM) strcat(pathInfo, "S");
	if(newPathType & PATH_TYPE_CONFIG) strcat(pathInfo, "C");
	if(newPathType & PATH_TYPE_GAME) strcat(pathInfo, "G");
	if(newPathType & PATH_TYPE_TEMP) strcat(pathInfo, "T");
	if(newPathType & PATH_TYPE_ANY) strcat(pathInfo, "A");

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
