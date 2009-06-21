
/*
 *
 * file.cpp
 *
 * Created on the 3rd of February 2009 from parts of util.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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
#include <string.h>


File::File (char * fileName, bool write) {

	char *filePath;


	// Open the file from the current directory
	f = fopen(fileName, write ? "wb": "rb");

	// If that succeeded, done
	if (f) {

		//printf("Opened %s\n", fileName);

		return;

	}

	// Allocate space for the file path
	filePath = new char[strlen(path) + strlen(fileName) + 1];

	// Create the file path
	strcpy(filePath, path);
	strcat(filePath, fileName);

	// Open the file from the path
	f = fopen(filePath, write ? "wb": "rb");

	if (f == NULL) {

		printf("Could not open %s\n", fileName);

		delete[] filePath;

		throw E_FILE;

	}

	//printf("Opened %s\n", filePath);

	// Free the file path
	delete[] filePath;

	return;

}


File::~File () {

	fclose(f);

	return;

}

int File::getSize () {

	int pos, size;

	pos = ftell(f);

	fseek(f, 0, SEEK_END);

	size = ftell(f);

	fseek(f, pos, SEEK_SET);

	return size;

}

int File::tell () {

	return ftell(f);

}

void File::seek (int offset, bool reset) {

	fseek(f, offset, reset ? SEEK_SET: SEEK_CUR);

	return;

}

unsigned char File::loadChar () {

	return fgetc(f);

}


void File::storeChar (unsigned char val) {

	fputc(val, f);

	return;

}


unsigned short int File::loadShort () {

	unsigned short int val;

	val = fgetc(f);
	val += fgetc(f) << 8;

	return val;

}


void File::storeShort (unsigned short int val) {

	fputc(val & 255, f);
	fputc(val >> 8, f);

	return;

}


signed long int File::loadInt () {

	unsigned long int val;

	val = fgetc(f);
	val += fgetc(f) << 8;
	val += fgetc(f) << 16;
	val += fgetc(f) << 24;

	return *((signed long int *)&val);

}


void File::storeInt (signed long int val) {

	unsigned long int uval;

	uval = *((unsigned long int *)&val);

	fputc(uval & 255, f);
	fputc((uval >> 8) & 255, f);
	fputc((uval >> 16) & 255, f);
	fputc(uval >> 24, f);

	return;

}


unsigned char * File::loadBlock (int length) {

	unsigned char *buffer;

	buffer = new unsigned char[length];

	fread(buffer, 1, length, f);

	return buffer;

}


unsigned char * File::loadRLE (int length) {

	unsigned char *buffer;
	int rle, pos, byte, count, next;

	// Determine the offset that follows the block
	next = fgetc(f);
	next += fgetc(f) << 8;
	next += ftell(f);


	buffer = new unsigned char[length];

	pos = 0;

	while (pos < length) {

		rle = fgetc(f);

		if (rle > 127) {

			byte = fgetc(f);

			for (count = 0; count < (rle & 127); count++) {

				buffer[pos++] = byte;
				if (pos >= length) break;

			}

		} else if (rle > 0) {

			for (count = 0; count < rle; count++) {

				buffer[pos++] = fgetc(f);
				if (pos >= length) break;

			}

		} else break;

	}

	fseek(f, next, SEEK_SET);

	return buffer;

}


void File::skipRLE () {

	int next;

	next = fgetc(f);
	next += fgetc(f) << 8;

	fseek(f, next, SEEK_CUR);

	return;

}


char * File::loadString () {

	char *string;
	int length, count;

	length = fgetc(f);
	count = 0;

	if (length) {

		string = new char[length + 1];
		for (; count < length; count++) string[count] = fgetc(f);

	} else {

		// If the length is not given, assume it is an 8.3 file name
		string = new char[13];
		for (; count < 9; count++) {

			string[count] = fgetc(f);

			if (string[count] == '.') {

				string[++count] = fgetc(f);
				string[++count] = fgetc(f);
				string[++count] = fgetc(f);
				count++;

				break;

			}

		}


	}

	string[count] = 0;

	return string;

}

SDL_Surface * File::loadSurface (int width, int height) {

	return createSurface(loadRLE(width * height), width, height);

}


void File::loadPalette (SDL_Color *palette) {

	unsigned char *buffer;
	int count;

	buffer = loadRLE(768);

	for (count = 0; count < 256; count++) {

		// Palette entries are 6-bit
		// Shift them upwards to 8-bit, and fill in the lower 2 bits
		palette[count].r = (buffer[count * 3] << 2) + (buffer[count * 3] >> 4);
		palette[count].g = (buffer[(count * 3) + 1] << 2) +
			(buffer[(count * 3) + 1] >> 4);
		palette[count].b = (buffer[(count * 3) + 2] << 2) +
			(buffer[(count * 3) + 2] >> 4);

	}

	delete[] buffer;

	return;

}


