
/**
 *
 * @file sprite.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * - 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * - 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * - 26th July 2009: Created anim.cpp from parts of sprite.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2013 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#include "video.h"
#include "sprite.h"


/**
 * Create a sprite.
 */
Sprite::Sprite () {

	pixels = NULL;
	xOffset = 0;
	yOffset = 0;

}


/**
 * Delete the sprite.
 */
Sprite::~Sprite () {

	if (pixels) SDL_FreeSurface(pixels);

}


/**
 * Make the sprite blank.
 */
void Sprite::clearPixels () {

	unsigned char data;

	if (pixels) SDL_FreeSurface(pixels);

	data = 0;
	pixels = createSurface(&data, 1, 1);
	enableColorKey(pixels, 0);

}


void Sprite::setOffset (short int x, short int y) {

	xOffset = x;
	yOffset = y;

}


/**
 * Set new pixel data for the sprite.
 *
 * @param data The new pixel data
 * @param width The width of the sprite image
 * @param height The height of the sprite image
 * @param key The transparent pixel value
 */
void Sprite::setPixels (unsigned char *data, int width, int height, unsigned char key) {

	if (pixels) SDL_FreeSurface(pixels);

	pixels = createSurface(data, width, height);
	enableColorKey(pixels, key);

}


/**
 * Get the width of the sprite.
 *
 * @return The width
 */
int Sprite::getWidth () {

	return pixels->w;

}


/**
 * Get the height of the sprite.
 *
 * @return The height
 */
int Sprite::getHeight() {

	return pixels->h;

}


/**
 * Get the horizontal offset of the sprite.
 *
 * @return The horizontal offset
 */
int Sprite::getXOffset () {

	return xOffset;

}


/**
 * Get the vertical offset of the sprite.
 *
 * @return The vertical offset
 */
int Sprite::getYOffset () {

	return yOffset;

}


/**
 * Set the sprite's palette, or a portion thereof.
 *
 * @param palette New palette
 * @param start First colour to change
 * @param amount Number of colours to change
 */
void Sprite::setPalette (SDL_Color *palette, int start, int amount) {

	setLogicalPalette(pixels, palette + start, start, amount);

}


/**
 * Map the whole of the sprite's palette to one index.
 *
 * @param index The index to use
 */
void Sprite::flashPalette (int index) {

	SDL_Color palette[MAX_PALETTE_COLORS];
	for (int i = 0; i < MAX_PALETTE_COLORS; i++)
		palette[i].r = palette[i].g = palette[i].b = index;

	setLogicalPalette(pixels, palette, 0, MAX_PALETTE_COLORS);

}


/**
 * Restore the sprite's palette to its original state.
 */
void Sprite::restorePalette () {

	video.restoreSurfacePalette(pixels);

}


/**
 * Draw the sprite
 *
 * @param x The x-coordinate at which to draw the sprite
 * @param y The y-coordinate at which to draw the sprite
 * @param includeOffsets Whether or not to include the sprite's offsets
 */
void Sprite::draw (int x, int y, bool includeOffsets) {

	SDL_Rect dst;

	dst.x = x;
	dst.y = y;

	if (includeOffsets) {

		dst.x += xOffset;
		dst.y += yOffset;

	}

	SDL_BlitSurface(pixels, NULL, canvas, &dst);

}


/**
 * Draw the sprite scaled
 *
 * @param x The x-coordinate at which to draw the sprite
 * @param y The y-coordinate at which to draw the sprite
 * @param scale The amount by which to scale the sprite
 */
void Sprite::drawScaled (int x, int y, fixed scale) {

	int width, height, fullWidth, fullHeight;
	int dstX, dstY;
	int srcX, srcY;

	unsigned char key = getColorKey(pixels);

	fullWidth = FTOI(pixels->w * scale);
	if (x < -(fullWidth >> 1)) return; // Off-screen
	if (x + (fullWidth >> 1) > canvasW) width = canvasW + (fullWidth >> 1) - x;
	else width = fullWidth;

	fullHeight = FTOI(pixels->h * scale);
	if (y < -(fullHeight >> 1)) return; // Off-screen
	if (y + (fullHeight >> 1) > canvasH) height = canvasH + (fullHeight >> 1) - y;
	else height = fullHeight;

	if (SDL_MUSTLOCK(canvas)) SDL_LockSurface(canvas);

	if (y < (fullHeight >> 1)) {

		srcY = (fullHeight >> 1) - y;
		dstY = 0;

	} else {

		srcY = 0;
		dstY = y - (fullHeight >> 1);

	}

	while (srcY < height) {

		unsigned char* srcRow = static_cast<unsigned char*>(pixels->pixels) + (pixels->pitch * DIV(srcY, scale));
		unsigned char* dstRow = static_cast<unsigned char*>(canvas->pixels) + (canvas->pitch * dstY);

		if (x < (fullWidth >> 1)) {

			srcX = (fullWidth >> 1) - x;
			dstX = 0;

		} else {

			srcX = 0;
			dstX = x - (fullWidth >> 1);

		}

		while (srcX < width) {

			unsigned char pixel = srcRow[DIV(srcX, scale)];
			if (pixel != key) dstRow[dstX] = pixel;

			srcX++;
			dstX++;

		}

		srcY++;
		dstY++;

	}

	if (SDL_MUSTLOCK(canvas)) SDL_UnlockSurface(canvas);

}
