
/**
 *
 * @file font.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created font.c
 * - 3rd February 2009: Renamed font.c to font.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the loading, displaying and freeing of screen fonts.
 *
 */

#include "io/file.h"
#include "font.h"
#include "video.h"
#include "io/log.h"

#include <stb_rect_pack.h>
#include <string.h>

namespace {
	constexpr unsigned int INVALID_FONT_CHAR = -1;

	constexpr int normalPadding = 2;
	constexpr int sceneStringPadding = 1;
}

void Font::commonSetup() {
	isOk = false;
	lineHeight = 0;
	spaceWidth = 0;
	nCharacters = MAX_FONT_CHARS;
	memset(atlasRects, 0, sizeof(atlasRects));
	memset(map, INVALID_FONT_CHAR, sizeof(map));
}

void Font::cleanMapping() {
	// remove empty
	for (int i = 0; i < MAX_FONT_CHARS; i++) {
		if (map[i] >= static_cast<unsigned int>(nCharacters))
			map[i] = INVALID_FONT_CHAR;
	}
}

/**
 * Load a font from the given .0FN file.
 *
 * @param fileName Name of an .0FN file
 */
Font::Font (const char* fileName) {
	commonSetup();

	// Load font from a font file
	File* file = new File(fileName, PATH_TYPE_GAME);
	int fileSize = file->getSize();

	// Checking font file
	unsigned char *identifier1 = file->loadBlock(18);
	char identifier2 = file->loadChar();
	if (memcmp(identifier1, "Digital Dimensions", 18) != 0 || identifier2 != 0x1A) {
		LOG_ERROR("Font not valid!");
		delete[] identifier1;
		return;
	}
	delete[] identifier1;

	spaceWidth = file->loadChar();
	lineHeight = file->loadChar();
	LOG_MAX("spaceWidth: %d, lineHeight: %d", spaceWidth, lineHeight);

	// temporary character data
	SDL_Surface *chars[MAX_FONT_CHARS];
	stbrp_rect rects[MAX_FONT_CHARS];

	// Load characters
	for (int i = 0; i < MAX_FONT_CHARS; i++) {
		if (file->tell() >= fileSize) {
			nCharacters = i;
			LOG_TRACE("Loaded %d characters from font.", nCharacters);

			break;
		}

		int size = file->loadShort();
		int w = 0;
		int h = 0;

		if (size > 4) {
			unsigned char* pixels = file->loadRLE(size);

			int width = pixels[0]|pixels[1] << 8;
			int height = pixels[2]|pixels[3] << 8;

			if (size - 4 >= width * height) {
				chars[i] = createSurface(pixels + 4, width, height);

				w = width;
				h = height;
			}

			delete[] pixels;
		}

		// setup sizes for packing
		rects[i] = { i, w, h, 0, 0, 0 };

		// save size
		atlasRects[i] = { -1, -1, w, h };
	}

	delete file;

	// Pack all characters in a 128x128 pixels surface
	int aW = 128;
	int aH = 128;
	characterAtlas = createSurface(nullptr, aW, aH);
	enableColorKey(characterAtlas, 0);

	stbrp_context ctx;
	stbrp_node nodes[aW];
	stbrp_init_target(&ctx, aW, aH, nodes, aW);

	bool res = stbrp_pack_rects(&ctx, rects, nCharacters);
	if(res) {
		for (int i = 0; i < nCharacters; i++) {
			// save position
			atlasRects[i].x = rects[i].x;
			atlasRects[i].y = rects[i].y;

			// copy char to atlas
			if (rects[i].w > 0 && rects[i].h > 0)
				SDL_BlitSurface(chars[i], nullptr, characterAtlas, &atlasRects[i]);
		}
	} else
		LOG_WARN("Could not pack font atlas!");

	// Delete single char surfaces
	for (int i = 0; i < nCharacters; i++) {
		if(rects[i].w > 0 && rects[i].h > 0)
			SDL_FreeSurface(chars[i]);
	}

	// Create ASCII->font map
	map[33] = 107; // !
	map[34] = 116; // "
	map[36] = 63;  // $
	map[39] = 115; // '
	map[40] = 111; // (
	map[41] = 112; // )
	map[43] = 105; // +
	map[44] = 101; // ,
	map[45] = 104; // -
	map[46] = 102; // .
	map[47] = 108; // /
	for (int i = 48; i < 58; i++)
		map[i] = i + 5;  // Numbers
	map[58] = 114; // :
	map[59] = 113; // ;
	map[61] = 106; // =
	map[63] = 103; // ?
	for (int i = 65; i < 91; i++)
		map[i] = i - 38; // Upper-case letters
	for (int i = 97; i < 123; i++)
		map[i] = i - 96; // Lower-case letters

	cleanMapping();

	isOk = res;
}


/**
 * Create a font from the panel pixel data.
 *
 * @param pixels Panel pixel data
 * @param big Whether to use the small or the big font
 */
Font::Font (unsigned char* pixels, bool big) {
	commonSetup();

	int charWidth = 8;
	spaceWidth = 5;
	if (big) {
		nCharacters = 40;
		lineHeight = 8;
	} else {
		nCharacters = 13;
		lineHeight = 7;
	}

	// temporary character data
	SDL_Surface *chars[nCharacters];
	stbrp_rect rects[MAX_FONT_CHARS];

	unsigned char* chrPixels = new unsigned char[charWidth * lineHeight];

	for (int i = 0; i < nCharacters; i++) {
		// copy to atlas
		for (int y = 0; y < lineHeight; y++) {
			memcpy(chrPixels + (y * charWidth),
				pixels + (i * charWidth) + (y * SW), charWidth);
		}
		chars[i] = createSurface(chrPixels, charWidth, lineHeight);

		// setup sizes for packing
		rects[i] = { i, charWidth, lineHeight, 0, 0, 0 };

		// save size
		atlasRects[i] = { -1, -1, charWidth, lineHeight };
	}

	delete[] chrPixels;

	// Pack all characters in a surface
	int aW, aH;
	if(big) {
		// 56x48
		aW = 7 * charWidth;
		aH = 6 * lineHeight;
	} else {
		// 32x28
		aW = 4 * charWidth;
		aH = 4 * lineHeight;
	}

	characterAtlas = createSurface(nullptr, aW, aH);

	stbrp_context ctx;
	stbrp_node nodes[aW];
	stbrp_init_target(&ctx, aW, aH, nodes, aW);

	bool res = stbrp_pack_rects(&ctx, rects, nCharacters);
	if(res) {
		for (int i = 0; i < nCharacters; i++) {
			// save position
			atlasRects[i].x = rects[i].x;
			atlasRects[i].y = rects[i].y;

			// copy char to atlas
			SDL_BlitSurface(chars[i], nullptr, characterAtlas, &atlasRects[i]);
		}
	} else
		LOG_WARN("Could not pack font atlas!");

	// Delete single char surfaces
	for (int i = 0; i < nCharacters; i++)
		SDL_FreeSurface(chars[i]);

	if (big) enableColorKey(characterAtlas, 31);

	// Create ASCII->font map
	if (big) {
		// Goes " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-:."
		map[45] = 37; // -
		map[46] = 39; // .
		map[58] = 38; // :
		for (int i = 47; i < 58; i++)
			map[i] = i - 47; // Numbers
		for (int i = 64; i < 91; i++) {
			map[i] = i - 54; // Upper-case letters
			map[i+32] = map[i]; // Lower-case letters (copy)
		}
	} else {
		// Goes " 0123456789oo" (where oo = infinity)
		// Use :; to represent the infinity symbol
		for (int i = 47; i < 60; i++)
			map[i] = i - 47; // Numbers and :;
	}

	isOk = res;
}


/**
 * Load a font from a .000 file.
 *
 * @param bonus whether to use FONTS.000 or BONUS.000
 */
Font::Font (bool bonus) {
	commonSetup();

	// Load font from FONTS.000 or BONUS.000
	File* file = new File(bonus? "BONUS.000": "FONTS.000", PATH_TYPE_GAME);

	int fileSize = file->getSize();
	nCharacters = file->loadShort(256);

	if (bonus) {
		int nSprites = file->loadShort();
		nCharacters -= nSprites;

		// Skip sprites
		for (int i = 0; i < nSprites; i++) {
			file->seek(4, false);

			int width = file->loadShort();
			if (width == 0xFFFF) width = 0;

			file->seek((width << 2) + file->loadShort(), false);
		}
	}

	// temporary character data
	SDL_Surface *chars[MAX_FONT_CHARS];
	stbrp_rect rects[MAX_FONT_CHARS];

	// Load characters
	for (int i = 0; i < nCharacters; i++) {
		if (file->tell() >= fileSize) {
			nCharacters = i;
			LOG_TRACE("Loaded %d characters from font.", nCharacters);

			break;
		}

		int width = file->loadShort(SW);
		int height = file->loadShort(SH);

		// adjust
		if (bonus) width = (width + 3) & ~3;
		else width <<= 2;

		file->seek(4, false);

		unsigned char *pixels = file->loadPixels(width * height);

		chars[i] = createSurface(pixels, width, height);

		delete[] pixels;

		// setup sizes for packing
		rects[i] = { i, width, height, 0, 0, 0 };

		// save size
		atlasRects[i] = { -1, -1, width, height };
	}

	delete file;

	spaceWidth = 5;
	// use "A" as reference
	lineHeight = atlasRects[0].h;
	LOG_MAX("spaceWidth: %d, lineHeight: %d", spaceWidth, lineHeight);

	// Pack all characters in a 160x160 pixels surface
	int aW = 160;
	int aH = 160;
	characterAtlas = createSurface(nullptr, aW, aH);
	enableColorKey(characterAtlas, 254);

	stbrp_context ctx;
	stbrp_node nodes[aW];
	stbrp_init_target(&ctx, aW, aH, nodes, aW);

	bool res = stbrp_pack_rects(&ctx, rects, nCharacters);
	if(res) {
		for (int i = 0; i < nCharacters; i++) {
			// save position
			atlasRects[i].x = rects[i].x;
			atlasRects[i].y = rects[i].y;

			// copy char to atlas
			SDL_BlitSurface(chars[i], nullptr, characterAtlas, &atlasRects[i]);
		}
	} else
		LOG_WARN("Could not pack font atlas!");

	// Delete single char surfaces
	for (int i = 0; i < nCharacters; i++)
		SDL_FreeSurface(chars[i]);

	// Create ASCII->font map
	if (bonus) {
		map[42] = 37; // *
		map[46] = 39; // .
		map[47] = 38; // /
		map[58] = 36; // :
	} else {
		map[37] = 36; // %
	}
	for (int i = 48; i < 58; i++)
		map[i] = i - 22; // Numbers
	for (int i = 65; i < 91; i++) {
		map[i] = i - 65; // Upper-case letters
		map[i+32] = map[i]; // Lower-case letters (copy)
	}

	cleanMapping();

	isOk = res;
}


/**
 * Delete the font.
 */
Font::~Font () {
	SDL_FreeSurface(characterAtlas);
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
	if (!isOk) return x;

	// Determine the position at which to draw the first character
	int xOffset = x;
	int yOffset = y;

	// Go through each character of the string
	for (int i = 0; string[i]; i++) {
		if (string[i] == '\n') {
			// reset after line break
			xOffset = x;
			yOffset += lineHeight;
		} else {
			unsigned int c = map[int(string[i])];

			// skip spaces and invalid
			if(c == INVALID_FONT_CHAR) {
				xOffset += spaceWidth;
				#if DEBUG_FONTS
				if (string[i] != ' ') // log invalid
					LOG_MAX("Skipping char %d in %s at index %d", string[i], string, i);
				#endif
				continue;
			}

			// Determine the character's position on the screen
			SDL_Rect dst = { xOffset, yOffset, 0, 0 };

			// Draw the character to the screen
			SDL_BlitSurface(characterAtlas, &atlasRects[c], canvas, &dst);

			xOffset += atlasRects[c].w + normalPadding;
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
	if (!isOk) return x;

	// Determine the position at which to draw the first character
	int offset = x;

	// Go through each character of the string
	for (int i = 0; string[i]; i++) {

		// Determine the character's position on the screen
		SDL_Rect dst = { offset, y, 0, 0 };

		// use space for invalid characters
		if (string[i] >= nCharacters) {
			offset += spaceWidth;
			continue;
		}

		int c = string[i];

		// Draw the character to the screen
		SDL_BlitSurface(characterAtlas, &atlasRects[c], canvas, &dst);

		offset += atlasRects[c].w + sceneStringPadding;
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
	if (!isOk) return;

	SDL_Rect dst;

	// n being 0 is a special case. It must not be considered to be a trailing
	// zero, as these are not displayed.
	if (!n) {
		unsigned int c = map[int('0')];

		// Determine 0's position on the screen
		dst.y = y;
		dst.x = x - atlasRects[c].w;

		// Draw 0 to the screen
		SDL_BlitSurface(characterAtlas, &atlasRects[c], canvas, &dst);

		return;
	}

	// Determine the length of the number to be drawn
	int count;
	if (n > 0) count = n;
	else count = -n;

	// Determine the position at which to draw the lowest digit
	int offset = x;

	while (count) {
		unsigned int c = map[int('0' + (count % 10))];

		offset -= atlasRects[c].w;

		// Determine the digit's position on the screen
		dst.y = y;
		dst.x = offset;

		// Draw the digit to the screen
		SDL_BlitSurface(characterAtlas, &atlasRects[c], canvas, &dst);

		count /= 10;
	}

	// If needed, draw the negative sign
	if (n < 0) {
		unsigned int c = map[int('-')];

		// Determine the negative sign's position on the screen
		dst.y = y;
		dst.x = offset - atlasRects[c].w;

		// Draw the negative sign to the screen
		SDL_BlitSurface(characterAtlas, &atlasRects[c], canvas, &dst);
	}
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
	if (!isOk) return;

	SDL_Color palette[MAX_PALETTE_COLORS];

	for (int i = 0; i < length; i++) {
		palette[i].r = palette[i].g = palette[i].b = (i * newLength / length) + newStart;
	}

	setLogicalPalette(characterAtlas, palette, start, length);
}


/**
 * Restore a palette to its original state.
 */
void Font::restorePalette () {
	if (!isOk) return;

	video.restoreSurfacePalette(characterAtlas);
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
 * Get the width of a space.
 *
 * @return The width
 */
int Font::getSpaceWidth () {

	return spaceWidth;

}


/**
 * Get the width of a single line of a given string.
 *
 * @param string The string to measure
 *
 * @return The width
 */
int Font::getStringWidth (const char *string) {
	if (!isOk) return 0;

	int stringWidth = 0;

	// Go through each character of the string
	for (int i = 0; string[i]; i++) {
		// Only get the width of the first line
		if (string[i] == '\n') return stringWidth;

		unsigned int c = map[int(string[i])];

		// skip spaces
		if(c == INVALID_FONT_CHAR) {
			stringWidth += spaceWidth;
			continue;
		}

		stringWidth += atlasRects[c].w + normalPadding;
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
	if (!isOk) return 0;

	int stringWidth = 0;

	// Go through each character of the string
	for (int i = 0; string[i]; i++) {

		// use space for invalid characters
		if (string[i] >= nCharacters) {
			stringWidth += spaceWidth;
			continue;
		}

		stringWidth += atlasRects[string[i]].w + sceneStringPadding;
	}

	return stringWidth;
}

#ifdef DEBUG_FONTS

/**
 * Save the generated font atlas to a BMP image.
 *
 * @param fileName File to save to
 */
void Font::saveAtlasAsBMP(const char *fileName) {
	if (!isOk) {
		LOG_WARN("Not saving empty font atlas!");
		return;
	}

	video.restoreSurfacePalette(characterAtlas);
	SDL_SaveBMP(characterAtlas, fileName);
}

#endif
