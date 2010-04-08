
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

	stream.close();

#ifdef VERBOSE
	log("Closed file", filePath);
#endif

	delete[] filePath;

	return;

}


bool File::open (const char* path, const char* name, bool write) {

	// Create the file path for the given directory
	filePath = createString(path, name);

	// Open the file from the path
	stream.clear();
	stream.open(filePath, (write ? std::ios::out: std::ios::in) | std::ios::binary);

	if (stream.is_open() && stream.good()) {

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

	pos = stream.tellg();

	stream.seekg(0, std::ios::end);

	size = stream.tellg();

	stream.seekg(pos, std::ios::beg);

	return size;

}

int File::tell () {

	return stream.tellg();

}

void File::seek (int offset, bool reset) {

	stream.seekg(offset, reset ? std::ios::beg: std::ios::cur);

	return;

}

unsigned char File::loadChar () {

	return stream.get();

}


void File::storeChar (unsigned char val) {

	stream.put(val);

	return;

}


unsigned short int File::loadShort () {

	unsigned short int val;

	val = stream.get();
	val += stream.get() << 8;

	return val;

}


void File::storeShort (unsigned short int val) {

	stream.put(val & 255);
	stream.put(val >> 8);

	return;

}


signed long int File::loadInt () {

	unsigned long int val;

	val = stream.get();
	val += stream.get() << 8;
	val += stream.get() << 16;
	val += stream.get() << 24;

	return *((signed long int *)&val);

}


void File::storeInt (signed long int val) {

	unsigned long int uval;

	uval = *((unsigned long int *)&val);

	stream.put(uval & 255);
	stream.put((uval >> 8) & 255);
	stream.put((uval >> 16) & 255);
	stream.put(uval >> 24);

	return;

}


unsigned char* File::loadBlock (int length) {

	unsigned char *buffer;

	buffer = new unsigned char[length];

	stream.read((char *)buffer, length);

	return buffer;

}


unsigned char* File::loadRLE (int length) {

	unsigned char* buffer;
	int rle, pos, byte, count, next;

	// Determine the offset that follows the block
	next = stream.get();
	next += stream.get() << 8;
	next += stream.tellg();

	buffer = new unsigned char[length];

	pos = 0;

	while (pos < length) {

		rle = stream.get();

		if (rle & 128) {

			byte = stream.get();

			for (count = 0; count < (rle & 127); count++) {

				buffer[pos++] = byte;
				if (pos >= length) break;

			}

		} else if (rle) {

			for (count = 0; count < rle; count++) {

				buffer[pos++] = stream.get();
				if (pos >= length) break;

			}

		} else buffer[pos++] = stream.get();

	}

	stream.seekg(next, std::ios::beg);

	return buffer;

}


void File::skipRLE () {

	int next;

	next = stream.get();
	next += stream.get() << 8;

	stream.seekg(next, std::ios::cur);

	return;

}


char* File::loadString () {

	char* string;
	int length, count;

	length = stream.get();

	if (length) {

		string = new char[length + 1];
		stream.read(string, length);

	} else {

		// If the length is not given, assume it is an 8.3 file name
		string = new char[13];

		for (count = 0; count < 9; count++) {

			string[count] = stream.get();

			if (string[count] == '.') {

				string[++count] = stream.get();
				string[++count] = stream.get();
				string[++count] = stream.get();
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


void File::loadPalette (SDL_Color* palette) {

	unsigned char* buffer;
	int count;

	buffer = loadRLE(768);

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


