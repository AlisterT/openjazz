
/*
 *
 * file.cpp
 *
 * 3rd February 2009: Created file.cpp from parts of util.cpp
 *
 * Part of the OpenJazz project
 *
 *
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


#include "file.h"

#include "io/gfx/video.h"
#include "util.h"

#include <string.h>
#include <zlib.h>


File::File (const char* name, bool write) {

	Path* path;

	// Try opening the file from all the available directories

	path = firstPath;

	while (path) {

		if (open(path->path, name, write)) return;
		path = path->next;

	}

	log("Could not open file", name);

	throw E_FILE;

}


File::~File () {

	fclose(file);

#ifdef VERBOSE
	log("Closed file", filePath);
#endif

	delete[] filePath;

	return;

}


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


int File::getSize () {

	int pos, size;

	pos = ftell(file);

	fseek(file, 0, SEEK_END);

	size = ftell(file);

	fseek(file, pos, SEEK_SET);

	return size;

}

int File::tell () {

	return ftell(file);

}

void File::seek (int offset, bool reset) {

	fseek(file, offset, reset ? SEEK_SET: SEEK_CUR);

	return;

}

unsigned char File::loadChar () {

	return fgetc(file);

}


void File::storeChar (unsigned char val) {

	fputc(val, file);

	return;

}


unsigned short int File::loadShort () {

	unsigned short int val;

	val = fgetc(file);
	val += fgetc(file) << 8;

	return val;

}


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


unsigned char * File::loadBlock (int length) {

	unsigned char *buffer;

	buffer = new unsigned char[length];

	fread(buffer, 1, length, file);

	return buffer;

}


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


void File::skipRLE () {

	int next;

	next = fgetc(file);
	next += fgetc(file) << 8;

	fseek(file, next, SEEK_CUR);

	return;

}


unsigned char* File::loadLZ (int compressedLength, int length) {

	unsigned char* compressedBuffer;
	unsigned char* buffer;

	compressedBuffer = loadBlock(compressedLength);

	buffer = new unsigned char[length];

	uncompress(buffer, (unsigned long int *)&length, compressedBuffer, compressedLength);

	delete[] compressedBuffer;

	return buffer;

}


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

SDL_Surface* File::loadSurface (int width, int height) {

	SDL_Surface* surface;
	unsigned char* pixels;

	pixels = loadRLE(width * height);

	surface = createSurface(pixels, width, height);

	delete[] pixels;

	return surface;

}


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


Path::Path (Path* newNext, char* newPath) {

	next = newNext;
	path = newPath;

	return;

}


Path::~Path () {

	if (next) delete next;
	delete[] path;

	return;

}


