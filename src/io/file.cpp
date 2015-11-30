
/**
 *
 * @file file.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed util.c to util.cpp
 * 3rd February 2009: Created file.cpp from parts of util.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with files.
 *
 */


#include "file.h"

#include "io/gfx/video.h"
#include "util.h"

#include <string.h>
#include <zlib.h>


/**
 * Try opening a file from the available paths
 *
 * @param name File name
 * @param write Whether or not the file can be written to
 */
File::File (const char* name, bool write) {

	Path* path;

	path = firstPath;

	while (path) {

		if (open(path->path, name, write)) return;
		path = path->next;

	}

	log("Could not open file", name);

	throw E_FILE;

}


/**
 * Delete the file object.
 */
File::~File () {

	fclose(file);

#ifdef VERBOSE
	log("Closed file", filePath);
#endif

	delete[] filePath;

	return;

}


/**
 * Try opening a file from the given path
 *
 * @param path Directory path
 * @param name File name
 * @param write Whether or not the file can be written to
 */
bool File::open (const char* path, const char* name, bool write) {

#if defined(UPPERCASE_FILENAMES) || defined(LOWERCASE_FILENAMES)
	int count;
#endif

	// Create the file path for the given directory
	filePath = createString(path, name);

#ifdef UPPERCASE_FILENAMES
	for (count = strlen(path); filePath[count]; count++) {

		if ((filePath[count] >= 97) && (filePath[count] <= 122)) filePath[count] -= 32;

	}
#endif

#ifdef LOWERCASE_FILENAMES
	for (count = strlen(path); filePath[count]; count++) {

		if ((filePath[count] >= 65) && (filePath[count] <= 90)) filePath[count] += 32;

	}
#endif

	// Open the file from the path
	file = fopen(filePath, write ? "wb": "rb");

	if (file) {

#ifdef VERBOSE
		log("Opened file", filePath);
#endif

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

	return;

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

	return;

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

		logError("Oversized value in file", filePath);

		return max;

	}

	return val;

}


void File::storeShort (unsigned short int val) {

	fputc(val & 255, file);
	fputc(val >> 8, file);

	return;

}


/**
 * Load a signed long int from the file.
 *
 * @return The value read
 */
signed long int File::loadInt () {

	unsigned long int val;

	val = fgetc(file);
	val += fgetc(file) << 8;
	val += fgetc(file) << 16;
	val += fgetc(file) << 24;

	return *((signed long int *)&val);

}


void File::storeInt (signed long int val) {

	unsigned long int uval;

	uval = *((unsigned long int *)&val);

	fputc(uval & 255, file);
	fputc((uval >> 8) & 255, file);
	fputc((uval >> 16) & 255, file);
	fputc(uval >> 24, file);

	return;

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

	fread(buffer, 1, length, file);

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

	unsigned char* buffer;
	int rle, pos, byte, count, next;

	// Determine the offset that follows the block
	next = fgetc(file);
	next += fgetc(file) << 8;
	next += ftell(file);

	buffer = new unsigned char[length];

	pos = 0;

	while (pos < length) {

		rle = fgetc(file);

		if (rle & 128) {

			byte = fgetc(file);

			for (count = 0; count < (rle & 127); count++) {

				buffer[pos++] = byte;
				if (pos >= length) break;

			}

		} else if (rle) {

			for (count = 0; count < rle; count++) {

				buffer[pos++] = fgetc(file);
				if (pos >= length) break;

			}

		} else buffer[pos++] = fgetc(file);

	}

	fseek(file, next, SEEK_SET);

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

	return;

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
 * Load a string from the file.
 *
 * @return The new string
 */
char * File::loadString () {

	char *string;
	int length, count;

	length = fgetc(file);

	if (length) {

		string = new char[length + 1];
		fread(string, 1, length, file);

	} else {

		// If the length is not given, assume it is an 8.3 file name
		string = new char[13];

		for (count = 0; count < 9; count++) {

			string[count] = fgetc(file);

			if (string[count] == '.') {

				string[++count] = fgetc(file);
				string[++count] = fgetc(file);
				string[++count] = fgetc(file);
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
 * Load RLE compressed graphical data from the file.
 *
 * @param width The width of the image to load
 * @param height The height of the image to load
 *
 * @return SDL surface containing the loaded image
 */
SDL_Surface* File::loadSurface (int width, int height) {

	SDL_Surface* surface;
	unsigned char* pixels;

	pixels = loadRLE(width * height);

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

	if (rle) buffer = loadRLE(768);
	else buffer = loadBlock(768);

	for (count = 0; count < 256; count++) {

		// Palette entries are 6-bit
		// Shift them upwards to 8-bit, and fill in the lower 2 bits
		palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
		palette[count].g = (buffer[(count * 3) + 1] << 2) + (buffer[(count * 3) + 1] >> 4);
		palette[count].b = (buffer[(count * 3) + 2] << 2) + (buffer[(count * 3) + 2] >> 4);

	}

	delete[] buffer;

	return;

}


/**
 * Create a new directory path object.
 *
 * @param newNext Next path
 * @param newPath The new path
 */
Path::Path (Path* newNext, char* newPath) {

	next = newNext;
	path = newPath;

	return;

}


/**
 * Delete the directory path object.
 */
Path::~Path () {

	if (next) delete next;
	delete[] path;

	return;

}


