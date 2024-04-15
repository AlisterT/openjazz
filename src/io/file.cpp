
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


#if !(defined(_WIN32) || defined(WII) || defined(PSP) || defined(__3DS__))
    #define UPPERCASE_FILENAMES
    #define LOWERCASE_FILENAMES
#endif

#if WANT_ZIP
#include <assetsys.h>

namespace {
	static assetsys_t* asset_context = nullptr;
}
#endif

/**
 * Try opening a file from the available paths.
 *
 * @param name File name
 * @param pathType Kind of directory
 * @param write Whether or not the file can be written to
 */
File::File (const char* name, int pathType, bool write) :
#if WANT_ZIP
	asset_loaded(false), asset_buffer(nullptr), asset_size(0), asset_pos(0),
#endif
	file(nullptr), filePath(nullptr) {
#if WANT_ZIP
	if(!write && pathType & PATH_TYPE_GAME) {
		assetsys_error_t err;
		assetsys_file_t asset_file;

		// Create the file path for the given directory
		filePath = createString("/GAME/", name);

		err = assetsys_file(asset_context, filePath, &asset_file);
		if(err != ASSETSYS_SUCCESS) {
			LOG_WARN("Could not find file: %s (%d)", name, err);
			delete[] filePath;
			throw E_FILE;
		}

        asset_size = assetsys_file_size(asset_context, asset_file);
		asset_buffer = new char[asset_size];
		const char *f = assetsys_file_to_path(asset_context, asset_file);

        int loaded_size = 0;
		err = assetsys_file_load(asset_context, asset_file, &loaded_size, asset_buffer, asset_size);
		if(err != ASSETSYS_SUCCESS || (size_t)loaded_size != asset_size) {
			delete[] asset_buffer;
			asset_size = 0;
			LOG_WARN("Could not open file: %s (%d)", f, err);
			delete[] filePath;
			throw E_FILE;
		}

		LOG_TRACE("Opened file: %s", f);
		asset_loaded = true;
		return;
	}
#endif
	forWriting = write;

	auto paths = gamePaths.getPaths(pathType);
	for (const auto &path : paths) {

		// only allow certain write paths
		if (!write || (pathType & (PATH_TYPE_CONFIG|PATH_TYPE_TEMP)) > 0) {
			if (open(path.path, name, write)) return;
		} else LOG_FATAL("Not allowed to write to %s", name);

	}

	LOG_WARN("Could not open file: %s", name);

	throw E_FILE;

}


/**
 * Delete the file object.
 */
File::~File () {
#if WANT_ZIP
	if (asset_loaded) {
		delete[] asset_buffer;
		asset_size = asset_pos = 0;
		asset_loaded = false;
	} else
#endif
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
bool File::open (std::string path, const char* name, bool write) {
	const char *fileMode = write ? "wb": "rb";

	// Create the file path for the given directory
	filePath = createString(path.c_str(), name);

	// Open the file from the path
	file = fopen(filePath, fileMode);

#ifdef UPPERCASE_FILENAMES
	if (!file) {

		uppercaseString(filePath + path.length());
		file = fopen(filePath, fileMode);

	}
#endif

#ifdef LOWERCASE_FILENAMES
	if (!file) {

		lowercaseString(filePath + path.length());
		file = fopen(filePath, fileMode);

	}
#endif

	if (file) {

		LOG_TRACE("Opened file: %s", filePath);

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
size_t File::getSize () {
#if WANT_ZIP
	if (asset_loaded) {
		return asset_size;
	}
#endif
	size_t pos = tell();
	fseek(file, 0, SEEK_END);
	size_t size = tell();
	seek(pos, true);

	return size;
}


/**
 * Get the current read/write location within the file.
 *
 * @return The current location
 */
size_t File::tell () {
#if WANT_ZIP
	if(asset_loaded) {
		return asset_pos;
	}
#endif
	return ftell(file);
}


/**
 * Set the read/write location within the file.
 *
 * @param offset The new offset
 * @param reset Whether to offset from the current location or the start of the file
 */
void File::seek (size_t offset, bool reset) {
#if WANT_ZIP
	if(asset_loaded) {
		asset_pos = reset ? offset : asset_pos + offset;
		return;
	}
#endif
	fseek(file, offset, reset ? SEEK_SET: SEEK_CUR);
}


/**
 * Load an unsigned char from the file.
 *
 * @return The value read
 */
unsigned char File::loadChar () {
#if WANT_ZIP
	if(asset_loaded) {
		if (asset_pos <= asset_size)
			return asset_buffer[asset_pos++];
		return EOF;
	}
#endif
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
	unsigned short int val = loadChar();
	val += loadChar() << 8;

	return val;
}


/**
 * Load an unsigned short int with an upper limit from the file.
 *
 * @return The value read
 */
unsigned short int File::loadShort (unsigned short int max) {
	unsigned short int val = loadShort();
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
	unsigned int val = loadChar();
	val += loadChar() << 8;
	val += loadChar() << 16;
	val += loadChar() << 24;

	return *((signed int *)&val);
}


void File::storeInt (signed int val) {
	unsigned int uval = *((unsigned int *)&val);

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
	unsigned char *buffer = new unsigned char[length];

#if WANT_ZIP
	if(asset_loaded) {
		int l = length;
		if(asset_pos + length > asset_size)
			l = asset_size - asset_pos;

		memcpy(buffer, (void*)(asset_buffer + asset_pos), l);

		asset_pos += l;
		return buffer;
	}
#endif
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
	size_t start = 0;
	int size = 0;

	if (checkSize) {
		// Determine the offset that follows the block
		size = loadShort();
		start = tell();
	}

	unsigned char* buffer = new unsigned char[length];

	int pos = 0;
	while (pos < length) {
		unsigned char code = loadChar();
		unsigned char amount = code & 127;

		if (code & 128) {
			unsigned char value = loadChar();

			if (pos + amount >= length) break;

			memset(buffer + pos, value, amount);
			pos += amount;
		} else if (amount) {
			if (pos + amount >= length) break;

#if WANT_ZIP
			for (int i = 0; i < amount; i++) {
				buffer[pos++] = loadChar();
			}
#else
			fread(buffer + pos, 1, amount, file);
			pos += amount;
#endif
		} else
			buffer[pos++] = loadChar();
	}

	if (checkSize) {
		if (tell() != start + size)
			LOG_DEBUG("RLE block has incorrect size: %zu vs. %d", tell() - start, size);

		seek(start + size, true);
	}

	return buffer;
}


/**
 * Skip past a block of RLE compressed data in the file.
 */
void File::skipRLE () {
	int next = loadShort();

	seek(next, false);
}


#ifdef ENABLE_JJ2
/**
 * Load a block of LZ compressed data from the file.
 *
 * @param compressedLength The length of the compressed block
 * @param length The length of the uncompressed block
 *
 * @return Buffer containing the uncompressed data
 */
unsigned char* File::loadLZ (int compressedLength, unsigned int length) {
	unsigned char* compressedBuffer = loadBlock(compressedLength);
	unsigned char* buffer = new unsigned char[length];

	int res = mz_uncompress(buffer, (unsigned long int *)&length, compressedBuffer, compressedLength);
	if(res != MZ_OK) {
		LOG_WARN("Could not uncompress Block: %s (%d)", mz_error(res), res);
		delete[] buffer;
		return nullptr;
	}

	delete[] compressedBuffer;
	return buffer;
}
#endif


/**
 * Load a string from the file.
 *
 * @return The new string
 */
char * File::loadString () {
	char *string;

	int length = loadChar();

	if (length) {

		string = new char[length + 1];

#if WANT_ZIP
		if(asset_loaded) {
			int l = length;
			if(asset_pos + length > asset_size)
				l = asset_size - asset_pos;

			memcpy(string, (void*)(asset_buffer + asset_pos), l);

			asset_pos += l;
			string[length] = 0;
			return string;
		}
#endif

		int res = fread(string, 1, length, file);

		if (res != length)
			LOG_ERROR("Could not read whole string (%d of %d bytes read)", res, length);

	} else {

		// If the length is not given, assume it is an 8.3 file name
		string = new char[13];
		LOG_TRACE("Assuming reading a filename from %s", filePath);

		int count;
		for (count = 0; count < 9; count++) {

			string[count] = loadChar();

			if (string[count] == '.') {

				string[++count] = loadChar();
				string[++count] = loadChar();
				string[++count] = loadChar();
				count++;

				break;

			}

		}

		length = count;

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

#if WANT_ZIP
	if(asset_loaded) {
		int l = length;
		if(asset_pos + length > asset_size)
			l = asset_size - asset_pos;

		memcpy(string, (void*)(asset_buffer + asset_pos), l);

		asset_pos += l;
		string[length] = 0;
		return string;
	}
#endif

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

		if (!(count & 3)) mask = loadChar();
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
			while (pixels[count] == key) pixels[count] = loadChar();

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

PathMgr::PathMgr() :
	paths(), config(""), temp("") {
#if WANT_ZIP
	asset_context = assetsys_create(nullptr);
#endif
}

#if WANT_ZIP
static void list_assets( assetsys_t* assetsys, char const* path, int indent ) {
    // Print folder names and recursively list assets
    for( int i = 0; i < assetsys_subdir_count( assetsys, path ); ++i ) {
        char const* subdir_name = assetsys_subdir_name( assetsys, path, i );
        for( int j = 0; j < indent; ++j ) printf( "  " );
        printf( "%s/\n", subdir_name );

        char const* subdir_path = assetsys_subdir_path( assetsys, path, i );
        list_assets( assetsys, subdir_path, indent + 1 );
    }

    // Print file names
    for( int i = 0; i < assetsys_file_count( assetsys, path ); ++i ) {
        char const* file_name = assetsys_file_name( assetsys, path, i );
        for( int j = 0; j < indent; ++j ) printf( "  " );
        printf( "%s\n", file_name );
    }
}
#endif

PathMgr::~PathMgr() {
#if WANT_ZIP

	//list_assets(asset_context, "/", 0);

	assetsys_destroy(asset_context);
#endif
}

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
	/*if (newPath[strlen(newPath) - 1] != OJ_DIR_SEP) {
		char* tmp = createString(newPath, OJ_DIR_SEP_STR);
		delete[] newPath;
		newPath = tmp;
	}*/

	// all paths need to be readable
	if (access(newPath, R_OK) != 0) {
		LOG_TRACE("Path '%s' is not readable, ignoring!", newPath);
		delete[] newPath;
		return false;
	}

	// ignore, if already present
	if(!config.empty()) newPathType &= ~PATH_TYPE_CONFIG;
	if(!temp.empty()) newPathType &= ~PATH_TYPE_TEMP;

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
		config = newPath;
		newPathType &= ~PATH_TYPE_CONFIG;

		LOG_DEBUG("Using directory '%s' for configuration.", newPath);
	}
	if (newPathType & PATH_TYPE_TEMP) {
		temp = newPath;
		newPathType &= ~PATH_TYPE_TEMP;

		LOG_DEBUG("Using directory '%s' for temporary files.", newPath);
	}

	// path could have been invalidated above
	if(newPathType == PATH_TYPE_INVALID) {
		delete[] newPath;
		return false;
	}

	// log info
	char pathInfo[4] = {};
	if(newPathType & PATH_TYPE_SYSTEM) strcat(pathInfo, "S");
	if(newPathType & PATH_TYPE_USER) strcat(pathInfo, "U");
	if(newPathType & PATH_TYPE_GAME) strcat(pathInfo, "G");

	LOG_DEBUG("Adding '%s' to the path list [%s]", newPath, pathInfo);

	// Finally add
	paths.emplace_back(Path(newPath, newPathType));
#if WANT_ZIP
	//newPath[strlen(newPath) - 1] = '\0';
	if(newPathType & PATH_TYPE_GAME) {
		assetsys_error_t err = assetsys_mount(asset_context, newPath, "/game");
		if(err != ASSETSYS_SUCCESS) {
			LOG_WARN("Could not add path %s to asset loader.", newPath);
		}
	}
#endif

	delete[] newPath;
	return true;
}

std::vector<Path> PathMgr::getPaths(int pathType) const {
	std::vector<Path> result;

	// special cases
	if(pathType == PATH_TYPE_CONFIG) {
		result.emplace_back(Path(config, pathType));
	} else if(pathType == PATH_TYPE_TEMP) {
		result.emplace_back(Path(temp, pathType));
	} else {
		// search list
		for (const auto &path : paths) {
			// skip other paths
			if ((path.pathType & pathType) != pathType) {
				continue;
			}

			result.emplace_back(path);
		}
	}

	return result;
}


/**
 * Create a new directory path object.
 *
 * @param newPath The new path
 */
Path::Path (const char* newPath, int newPathType) {

	path = std::string(newPath);
	pathType = newPathType;

}

Path::Path (std::string const &newPath, int newPathType) {

	path = newPath;
	pathType = newPathType;

}
