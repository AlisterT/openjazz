
/*
 *
 * font.cpp
 *
 * Created as font.c on the 23rd of August 2005
 * Renamed font.cpp on the 3rd of February 2009
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

/*
 * Deals with the loading, displaying and freeing of screen fonts.
 *
 */


#include "font.h"
#include "palette.h"
#include <string.h>


Font::Font (char * fileName) {

	File *file;
	unsigned char *pixels, *character;
	int rle, pos, byte, count, next, fileSize;
	int chr, width, height, y;

	// Load font from a font file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	file->seek(19, true);

	width = file->loadChar() << 2;
	h = file->loadChar() << 1;
	w = new unsigned char[128];
	pixels = new unsigned char[width * h * 128];
	memset(pixels, 0, width * h * 128);

	file->seek(23, true);

	w[0] = width >> 1;

	fileSize = file->getSize();

	for (chr = 1; chr < 128; chr++) {

		file->seek(2, false);
		next = file->loadChar();

		if (file->tell() > fileSize) break;

		next += file->loadChar() << 8;
		next += file->tell();

		file->seek(1, false);
		w[chr] = file->loadChar();
		file->seek(1, false);
		height = file->loadChar();

		character = new unsigned char[(w[chr] * height) + 1];

		pos = 0;

		while (pos < w[chr] * height) {

			rle = file->loadChar();

			if (rle > 127) {

				byte = file->loadChar();

				for (count = 0; count < (rle & 127); count++) {

					character[pos++] = byte;
					if (pos >= w[chr] * height) break;

				}

			} else if (rle > 0) {

				for (count = 0; count < rle; count++) {

					character[pos++] = file->loadChar();
					if (pos >= w[chr] * height) break;

				}

			} else break;

		}

		character[pos] = character[pos - 1];

		for (y = 0; y < height; y++)
			memcpy(pixels + (((chr * h) + y) * width),
				character + (y * w[chr]) + 1, w[chr]);

		delete[] character;

		w[chr] += 2;

		file->seek(next, true);

	}

	delete file;


	for (; chr < 128; chr++) w[chr] = width >> 1;

	surface = createSurface(pixels, width, h * 128);
	SDL_SetColorKey(surface, SDL_SRCCOLORKEY, 0);


	// Create ASCII->font map

	if (!strcmp(fileName, "fontmn1.0fn")) {

		for (count = 0; count < 48; count++) map[count] = 0;
		for (; count < 58; count++) map[count] = count - 8;
		for (; count < 65; count++) map[count] = 0;
		for (; count < 91; count++) map[count] = count - 64;
		for (; count < 97; count++) map[count] = 0;
		for (; count < 123; count++) map[count] = count - 96;
		for (; count < 128; count++) map[count] = 0;

	} else if (!strcmp(fileName, "fontmn2.0fn")) {

		for (count = 0; count < 43; count++) map[count] = 0;
		map[33] = 89;
		map[34] = 96;
		map[39] = 95;
		map[43] = 87;
		map[44] = 83;
		map[45] = 86;
		map[46] = 84;
		map[47] = 90;
		count = 48;
		for (; count < 58; count++) map[count] = count + 5;
		map[58] = 94;
		map[59] = 93;
		map[60] = 0;
		map[61] = 88;
		map[62] = 0;
		map[63] = 85;
		map[64] = 0;
		count = 65;
		for (; count < 91; count++) map[count] = count - 38;
		for (; count < 97; count++) map[count] = 0;
		for (; count < 123; count++) map[count] = count - 96;
		for (; count < 128; count++) map[count] = 0;

	} else {

		for (count = 0; count < 40; count++) map[count] = 0;
		for (; count < 42; count++) map[count] = count + 25;
		for (; count < 48; count++) map[count] = 0;
		map[44] = 101;
		map[45] = 104;
		map[46] = 102;
		for (; count < 58; count++) map[count] = count + 5;
		for (; count < 65; count++) map[count] = 0;
		map[63] = 103;
		for (; count < 91; count++) map[count] = count - 64;
		for (; count < 97; count++) map[count] = 0;
		for (; count < 123; count++) map[count] = count - 70;
		for (; count < 128; count++) map[count] = 0;

	}

	return;

}


Font::Font (File *file, bool big) {

	unsigned char *pixels;
	int rle, pos, index, count;

	// Load font from panel.000

	if (big) h = 8;
	else h = 7;

	pixels = new unsigned char[320 * h];

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
	while (pos < 320 * h) {

		rle = file->loadChar();

		if (rle >= 128) {

			index = file->loadChar();

			for (count = 0; count < (rle & 127); count++) {

				pixels[(pos & 7) + ((pos / 320) * 8) +
					(((pos % 320)>>3) * 8 * h)] = index;
				pos++;

			}

		} else if (rle > 0) {

			for (count = 0; count < rle; count++) {

				pixels[(pos & 7) + ((pos / 320) * 8) +
					(((pos % 320)>>3) * 8 * h)] = file->loadChar();
				pos++;

			}

		} else break;

	}


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

		// Set font dimensions
		w = new unsigned char[40];
		for (count = 0; count < 40; count++) w[count] = 8;

	} else {

		// Create ASCII->font map
		for (count = 0; count < 48; count++) map[count] = 12;
		// Use :; to represent the infinity symbol
		for (; count < 60; count++) map[count] = count - 48;
		for (; count < 128; count++) map[count] = 12;

		// Set font dimensions
		w = new unsigned char[13];
		for (count = 0; count < 13; count++) w[count] = 8;

	}

	surface = createSurface(pixels, 8, 40 * h);

	return;

}


Font::~Font () {

	SDL_FreeSurface(surface);
	delete[] w;

	return;

}



int Font::showString (char * s, int x, int y) {

	SDL_Rect src, dst;
	unsigned int count;
	int xOffset, yOffset;

	// Determine the characters' dimensions
	src.x = 0;
	src.h = h;

	// Determine the position at which to draw the first character
	xOffset = x;
	yOffset = y;

	// Go through each character of the string
	for (count = 0; count < strlen(s); count++) {

		if (s[count] == '\n') {

			xOffset = x;
			yOffset += h;

		} else {

			// Determine the character's position on the screen
			src.w = w[(int)(map[(int)(s[count])])];
			dst.y = yOffset;
			dst.x = xOffset;

			// Determine the character's position in the font
			if (s[count] >= 0) src.y = map[(int)(s[count])] * h;
			else src.y = 0;

			// Draw the character to the screen
			SDL_BlitSurface(surface, &src, screen, &dst);

			xOffset += w[(int)(map[(int)(s[count])])];

		}

	}

	return xOffset;

}



void Font::showNumber (int n, int x, int y) {

	SDL_Rect src, dst;
	int count, offset;

	// Determine the characters' dimensions
	src.x = 0;
	src.h = h;

	// n being 0 is a special case. It must not be considered to be a trailing
	// zero, as these are not displayed.
	if (!n) {

		// Determine 0's position on the screen
		src.w = w[(int)(map['0'])];
		dst.y = y;
		dst.x = x - src.w;

		// Determine 0's position in the font
		src.y = map['0'] * h;

		// Draw 0 to the screen
		SDL_BlitSurface(surface, &src, screen, &dst);

		return;

	}

	// Determine the length of the number to be drawn
	if (n > 0) count = n;
	else count = -n;

	// Determine the position at which to draw the lowest digit
	offset = x;

	while (count) {

		// Determine the digit's position on the screen
		src.w = w[(int)(map['0' + (count % 10)])];
		offset -= src.w;
		dst.y = y;
		dst.x = offset;

		// Determine the digit's position in the font
		src.y = map['0' + (count % 10)] * h;

		// Draw the digit to the screen
		SDL_BlitSurface(surface, &src, screen, &dst);

		count /= 10;

	}

	// If needed, draw the negative sign
	if (n < 0) {

		// Determine the negative sign's position on the screen
		src.w = w[(int)(map['-'])];
		dst.y = y;
		dst.x = offset - src.w;

		// Determine the negative sign's position on the screen
		src.y = map['-'] * h;

		// Draw the negative sign to the screen
		SDL_BlitSurface(surface, &src, screen, &dst);

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

	SDL_SetPalette(surface, SDL_LOGPAL, palette, start, length);

	return;

}


void Font::restorePalette () {

	::restorePalette(surface);

	return;

}


