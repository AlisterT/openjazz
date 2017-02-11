
/**
 *
 * @file font.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created font.c
 * 3rd February 2009: Renamed font.c to font.cpp
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
 * Deals with the loading, displaying and freeing of screen fonts.
 *
 */


#include "../file.h"
#include "font.h"
#include "video.h"

#include <string.h>


/**
 * Load a font from the given .0FN file.
 *
 * @param fileName Name of an .0FN file
 */
Font::Font (const char* fileName) {

	File* file;
	unsigned char* pixels;
	unsigned char* blank;
	int fileSize;
	int count, size, width, height;

	// Load font from a font file

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	fileSize = file->getSize();

	nCharacters = 128;


	file->seek(20, true);
	lineHeight = file->loadChar() << 1;


	// Create blank character data

	blank = new unsigned char[3];
	memset(blank, 0, 3);


	// Load characters

	for (count = 0; count < 128; count++) {

		if (file->tell() >= fileSize) {

			nCharacters = count;

			break;

		}

		size = file->loadShort();

		if (size > 4) {

			pixels = file->loadRLE(size);

			width = pixels[0];
			width += pixels[1] << 8;
			height = pixels[2];
			height += pixels[3] << 8;

			if (size - 4 >= width * height)
				characters[count] = createSurface(pixels + 4, width, height);
			else
				characters[count] = createSurface(blank, 3, 1);

			delete[] pixels;

		} else characters[count] = createSurface(blank, 3, 1);

		SDL_SetColorKey(characters[count], SDL_SRCCOLORKEY, 0);

	}

	delete[] blank;

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


/**
 * Create a font from the panel pixel data.
 *
 * @param pixels Panel pixel data
 * @param big Whether to use the small or the big font
 */
Font::Font (unsigned char* pixels, bool big) {

	unsigned char* chrPixels;
	int count, y;

	if (big) lineHeight = 8;
	else lineHeight = 7;

	chrPixels = new unsigned char[8 * lineHeight];

	for (count = 0; count < 40; count++) {

		for (y = 0; y < lineHeight; y++)
			memcpy(chrPixels + (y * 8), pixels + (count * 8) + (y * SW), 8);

		characters[count] = createSurface(chrPixels, 8, lineHeight);

		if (big) SDL_SetColorKey(characters[count], SDL_SRCCOLORKEY, 31);

	}

	nCharacters= 40;

	delete[] chrPixels;


	// Create ASCII->font map

	if (big) {

		// Goes " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-:."

		for (count = 0; count < 45; count++) map[count] = 0;
		map[count++] = 37;
		map[count++] = 39;
		for (; count < 48; count++) map[count] = 0;
		for (; count < 58; count++) map[count] = count - 47; // Numbers
		map[count++] = 38;
		for (; count < 65; count++) map[count] = 0;
		for (; count < 91; count++) map[count] = count - 54; // Upper-case letters
		for (; count < 97; count++) map[count] = 0;
		for (; count < 123; count++) map[count] = count - 86; // Lower-case letters
		for (; count < 128; count++) map[count] = 0;

	} else {

		// Goes " 0123456789oo" (where oo = infinity)
		// Use :; to represent the infinity symbol

		for (count = 0; count < 48; count++) map[count] = 0;
		for (; count < 60; count++) map[count] = count - 47; // Numbers and :;
		for (; count < 128; count++) map[count] = 0;

	}

	return;

}


/**
 * Load a font from a .000 file.
 *
 * @param bonus whether to use FONTS.000 or BONUS.000
 */
Font::Font (bool bonus) {

	File* file;
	unsigned char* pixels;
	int fileSize;
	int count, width, height;

	// Load font from FONTS.000 or BONUS.000

	try {

		file = new File(bonus? F_BONUS: F_FONTS, false);

	} catch (int e) {

		throw e;

	}


	fileSize = file->getSize();

	nCharacters = file->loadShort(256);

	if (bonus) {

		count = file->loadShort();
		nCharacters -= count;

		// Skip sprites

		for (; count > 0; count--) {

			file->seek(4, false);

			width = file->loadShort();
			if (width == 0xFFFF) width = 0;

			file->seek((width << 2) + file->loadShort(), false);

		}

	}

	// Load characters

	for (count = 0; count < nCharacters; count++) {

		if (file->tell() >= fileSize) {

			nCharacters = count;

			break;

		}

		width = file->loadShort(SW);
		height = file->loadShort(SH);

		if (bonus) width = (width + 3) & ~3;
		else width <<= 2;

		file->seek(4, false);

		pixels = file->loadPixels(width * height);

		characters[count] = createSurface(pixels, width, height);
		SDL_SetColorKey(characters[count], SDL_SRCCOLORKEY, 254);

		delete[] pixels;

	}

	delete file;

	lineHeight = characters[0]->h;


	// Create blank character data

	pixels = new unsigned char[3];
	memset(pixels, 254, 3);
	characters[nCharacters] = createSurface(pixels, 3, 1);
	SDL_SetColorKey(characters[nCharacters], SDL_SRCCOLORKEY, 254);
	delete[] pixels;


	// Create ASCII->font map

	count = 0;

	if (bonus) {

		for (; count < 42; count++) map[count] = nCharacters;
		map[count++] = 37; // *
		for (; count < 46; count++) map[count] = nCharacters;
		map[count++] = 39; // .
		map[count++] = 38; // /
		for (; count < 59; count++) map[count] = count - 22; // Numbers and :

	} else {

		for (; count < 37; count++) map[count] = nCharacters;
		map[count++] = 36; // %
		for (; count < 48; count++) map[count] = nCharacters;
		for (; count < 58; count++) map[count] = count - 22; // Numbers

	}

	for (; count < 65; count++) map[count] = nCharacters;
	for (; count < 91; count++) map[count] = count - 65; // Upper-case letters
	for (; count < 97; count++) map[count] = nCharacters;
	for (; count < 123; count++) map[count] = count - 97; // Lower-case letters
	for (; count < 128; count++) map[count] = nCharacters;

	nCharacters++;

	for (count = 0; count < 128; count++) {

		if (map[count] >= nCharacters) map[count] = 0;

	}

	return;

}


/**
 * Delete the font.
 */
Font::~Font () {

	int count;

	for (count = 0; count < nCharacters; count++) SDL_FreeSurface(characters[count]);

	return;

}


/**
 * Draw a string using the font.
 *
 * @param string The string to draw
 * @param x The x-coordinate at which to draw the string
 * @param y The y-coordinate at which to draw the string
 *
 * @return The x-coordinate of the end of the string
 */
int Font::showString (const char* string, int x, int y) {

	SDL_Surface* surface;
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


/**
 * Draw a JJ1 cutscene string using the font.
 *
 * @param string The JJ1 cutstring to draw
 * @param x The x-coordinate at which to draw the string
 * @param y The y-coordinate at which to draw the string
 *
 * @return The x-coordinate of the end of the string
 */
int Font::showSceneString (const unsigned char* string, int x, int y) {

	SDL_Surface* surface;
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


/**
 * Draw a number using the font.
 *
 * @param n The number to draw
 * @param x The x-coordinate at which to draw the number
 * @param y The y-coordinate at which to draw the number
 *
 * @return The x-coordinate of the end of the number
 */
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


/**
 * Map a range of palette indices to another range
 *
 * @param start Start of original range
 * @param length Span of original range
 * @param newStart Start of new range
 * @param newLength Span of new range
 */
void Font::mapPalette (int start, int length, int newStart, int newLength) {

	SDL_Color palette[256];
	int count;

	for (count = 0; count < length; count++)
		palette[count].r = palette[count].g = palette[count].b =
			(count * newLength / length) + newStart;

	for (count = 0; count < nCharacters; count++)
		SDL_SetPalette(characters[count], SDL_LOGPAL, palette, start, length);

	return;

}


/**
 * Restore a palette to its original state.
 */
void Font::restorePalette () {

	int count;

	for (count = 0; count < nCharacters; count++)
		video.restoreSurfacePalette(characters[count]);

	return;

}


/**
 * Get the height of a single line of any text.
 *
 * @return The height
 */
int Font::getHeight () {

	return lineHeight;

}


/**
 * Get the width of a single line of a given string.
 *
 * @param string The string to measure
 *
 * @return The width
 */
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


/**
 * Get the width of a single line of a given JJ1 cutscene string.
 *
 * @param string The string to measure
 *
 * @return The width
 */
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


