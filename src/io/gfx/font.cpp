
/*
 *
 * font.cpp
 *
 * 23rd August 2005: Created font.c
 * 3rd February 2009: Renamed font.c to font.cpp
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

/*
 * Deals with the loading, displaying and freeing of screen fonts.
 *
 */


#include "../file.h"
#include "font.h"
#include "video.h"

#include <string.h>


Font::Font (const char * fileName) {

	File *file;
	unsigned char *pixels;
	int fileSize;
	int count, size, width, height;

	// Load font from a font file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}


	file->seek(20, true);
	lineHeight = file->loadChar() << 1;


	// Create the first character

	pixels = new unsigned char[3 * lineHeight];

	memset(pixels, 0, 3 * lineHeight);
	characters[0] = createSurface(pixels, 3, lineHeight);
	SDL_SetColorKey(characters[0], SDL_SRCCOLORKEY, 0);

	delete[] pixels;


	// Create remaining characters

	fileSize = file->getSize();
	nCharacters = 128;

	file->seek(23, true);

	for (count = 1; count < 128; count++) {

		size = file->loadShort();

		if (file->tell() >= fileSize) {

			nCharacters = count;

			break;

		}

		if (size) {

			pixels = file->loadRLE(size);

			width = pixels[0];
			width += pixels[1] << 8;
			height = pixels[2];
			height += pixels[3] << 8;

			characters[count] = createSurface(pixels + 4, width, height);
			SDL_SetColorKey(characters[count], SDL_SRCCOLORKEY, 0);

			delete[] pixels;

		} else characters[count] = createSurface(NULL, 1, 1);

	}

	delete file;


	// Create ASCII->font map

	for (count = 0; count < 33; count++) map[count] = 0;
	map[33] = 107; // !
	map[34] = 116; // "
	map[35] = 0; // #
	map[36] = 63; // $
	map[37] = 0; // %
	map[38] = 0; // &
	map[39] = 115; // '
	map[40] = 111; // (
	map[41] = 112; // )
	map[42] = 0; // *
	map[43] = 105; // +
	map[44] = 101; // ,
	map[45] = 104; // -
	map[46] = 102; // .
	map[47] = 108; // /
	for (count = 48; count < 58; count++) map[count] = count + 5;  // Numbers
	map[58] = 114; // :
	map[59] = 113; // ;
	map[60] = 0; // <
	map[61] = 106; // =
	map[62] = 0; // >
	map[63] = 103; // ?
	map[64] = 0; // @
	for (count = 65; count < 91; count++) map[count] = count - 38; // Upper-case letters
	for (; count < 97; count++) map[count] = 0;
	for (; count < 123; count++) map[count] = count - 96; // Lower-case letters
	for (; count < 128; count++) map[count] = 0;

	for (count = 0; count < 128; count++) {

		if (map[count] >= nCharacters) map[count] = 0;

	}

	return;

}


Font::Font (File *file, bool big) {

	unsigned char *pixels;
	int rle, pos, index, count;

	// Load font from panel.000

	if (big) lineHeight = 8;
	else lineHeight = 7;

	pixels = new unsigned char[320 * lineHeight];

	if (big) {

		// Load the large panel font
		// Starts at 4691 and goes 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-:.

		pixels[0] = BLACK;

		pos = 1;

		file->seek(4691, true);

	} else {

		// Load the small panel font
		// Starts at 6975 and goes 0123456789oo (where oo = infinity)

		pos = 0;

		file->seek(6975, true);

	}

	// RLE decompression and horizontal to vertical character rearrangement
	while (pos < 320 * lineHeight) {

		rle = file->loadChar();

		if (rle >= 128) {

			index = file->loadChar();

			for (count = 0; count < (rle & 127); count++) {

				pixels[(pos & 7) + ((pos / 320) * 8) +
					(((pos % 320)>>3) * 8 * lineHeight)] = index;
				pos++;

			}

		} else if (rle > 0) {

			for (count = 0; count < rle; count++) {

				pixels[(pos & 7) + ((pos / 320) * 8) +
					(((pos % 320)>>3) * 8 * lineHeight)] = file->loadChar();
				pos++;

			}

		} else break;

	}

	for (count = 0; count < 40; count++)
		characters[count] = createSurface(pixels + (count * 8 * lineHeight), 8, lineHeight);

	nCharacters= 40;

	delete[] pixels;


	if (big) {

		// Create ASCII->font map
		for (count = 0; count < 45; count++) map[count] = 39;
		map[45] = 36;
		map[46] = 38;
		for (count = 47; count < 48; count++) map[count] = 39;
		for (; count < 58; count++) map[count] = count - 48;
		map[58] = 37;
		for (count = 59; count < 65; count++) map[count] = 39;
		for (; count < 91; count++) map[count] = count - 55;
		for (; count < 97; count++) map[count] = 39;
		for (; count < 123; count++) map[count] = count - 87;
		for (; count < 128; count++) map[count] = 39;

	} else {

		// Create ASCII->font map
		for (count = 0; count < 48; count++) map[count] = 12;
		// Use :; to represent the infinity symbol
		for (; count < 60; count++) map[count] = count - 48;
		for (; count < 128; count++) map[count] = 12;

	}

	return;

}


Font::~Font () {

	int count;

	for (count = 0; count < nCharacters; count++) SDL_FreeSurface(characters[count]);

	return;

}


int Font::showString (const char *string, int x, int y) {

	SDL_Surface *surface;
	SDL_Rect dst;
	unsigned int count;
	int xOffset, yOffset;

	// Determine the position at which to draw the first character
	xOffset = x;
	yOffset = y;

	// Go through each character of the string
	for (count = 0; string[count]; count++) {

		if (string[count] == '\n') {

			xOffset = x;
			yOffset += lineHeight;

		} else {

			// Determine the character's position on the screen
			dst.y = yOffset;
			dst.x = xOffset;

			// Determine the character's surface
			surface = characters[int(map[int(string[count])])];

			// Draw the character to the screen
			SDL_BlitSurface(surface, NULL, canvas, &dst);

			xOffset += surface->w + 2;

		}

	}

	return xOffset;

}


int Font::showSceneString (const unsigned char *string, int x, int y) {

	SDL_Surface *surface;
	SDL_Rect dst;
	unsigned int count;
	int offset;

	// Determine the position at which to draw the first character
	offset = x;

	// Go through each character of the string
	for (count = 0; string[count]; count++) {

		// Determine the character's position on the screen
		dst.y = y;
		dst.x = offset;

		// Determine the character's surface
		if (string[count] < nCharacters) surface = characters[int(string[count])];
		else surface = characters[0];

		// Draw the character to the screen
		SDL_BlitSurface(surface, NULL, canvas, &dst);

		offset += surface->w + 1;

	}

	return offset;

}


void Font::showNumber (int n, int x, int y) {

	SDL_Surface *surface;
	SDL_Rect dst;
	int count, offset;

	// n being 0 is a special case. It must not be considered to be a trailing
	// zero, as these are not displayed.
	if (!n) {

		// Determine 0's surface
		surface = characters[int(map[int('0')])];

		// Determine 0's position on the screen
		dst.y = y;
		dst.x = x - surface->w;

		// Draw 0 to the screen
		SDL_BlitSurface(surface, NULL, canvas, &dst);

		return;

	}

	// Determine the length of the number to be drawn
	if (n > 0) count = n;
	else count = -n;

	// Determine the position at which to draw the lowest digit
	offset = x;

	while (count) {

		// Determine the digit's surface
		surface = characters[int(map['0' + (count % 10)])];

		offset -= surface->w;

		// Determine the digit's position on the screen
		dst.y = y;
		dst.x = offset;

		// Draw the digit to the screen
		SDL_BlitSurface(surface, NULL, canvas, &dst);

		count /= 10;

	}

	// If needed, draw the negative sign
	if (n < 0) {

		// Determine the negative sign's surface
		surface = characters[int(map[int('-')])];

		// Determine the negative sign's position on the screen
		dst.y = y;
		dst.x = offset - surface->w;

		// Draw the negative sign to the screen
		SDL_BlitSurface(surface, NULL, canvas, &dst);

	}

	return;

}


void Font::mapPalette (int start, int length, int newStart, int newLength) {

	SDL_Color palette[256];
	int count;

	// Map a range of palette indices to another range

	for (count = 0; count < length; count++)
		palette[count].r = palette[count].g = palette[count].b =
			(count * newLength / length) + newStart;

	for (count = 0; count < nCharacters; count++)
		SDL_SetPalette(characters[count], SDL_LOGPAL, palette, start, length);

	return;

}


void Font::restorePalette () {

	int count;

	for (count = 0; count < nCharacters; count++)
		::restorePalette(characters[count]);

	return;

}


int Font::getHeight () {

	return lineHeight;

}


int Font::getStringWidth (const char *string) {

	int count;
	int stringWidth = 0;

	// Go through each character of the string
	for (count = 0; string[count]; count++) {

		// Only get the width of the first line
		if (string[count] == '\n') return stringWidth;

		stringWidth += characters[int(map[int(string[count])])]->w + 2;

	}

	return stringWidth;

}


int Font::getSceneStringWidth (const unsigned char *string) {

	int count;
	int stringWidth = 0;

	// Go through each character of the string
	for (count = 0; string[count]; count++) {

		if (string[count] < nCharacters) stringWidth += characters[int(string[count])]->w + 1;
		else stringWidth += characters[0]->w + 1;

	}

	return stringWidth;

}


