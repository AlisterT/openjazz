
/**
 *
 * @file sprite.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 1st January 2006: Created events.c from parts of level.c
 * 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 26th July 2009: Created anim.cpp from parts of sprite.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

	return;

}


/**
 * Delete the sprite.
 */
Sprite::~Sprite () {

	if (pixels) SDL_FreeSurface(pixels);

	return;

}


/**
 * Make the sprite blank.
 */
void Sprite::clearPixels () {

	unsigned char data;

	if (pixels) SDL_FreeSurface(pixels);

	data = 0;
	pixels = createSurface(&data, 1, 1);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, 0);

	return;

}


void Sprite::setOffset (short int x, short int y) {

	xOffset = x;
	yOffset = y;

	return;

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
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, key);

	return;

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

	SDL_SetPalette(pixels, SDL_LOGPAL, palette + start, start, amount);

	return;

}


/**
 * Map the whole of the sprite's palette to one index.
 *
 * @param index The index to use
 */
void Sprite::flashPalette (int index) {

	SDL_Color palette[256];
	int count;

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = index;

	SDL_SetPalette(pixels, SDL_LOGPAL, palette, 0, 256);

	return;

}


/**
 * Restore the sprite's palette to its original state.
 */
void Sprite::restorePalette () {

	video.restoreSurfacePalette(pixels);

	return;

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

	return;

}


/**
 * Draw the sprite scaled
 *
 * @param x The x-coordinate at which to draw the sprite
 * @param y The y-coordinate at which to draw the sprite
 * @param scale The amount by which to scale the sprite
 */
void Sprite::drawScaled (int x, int y, fixed scale) {

	unsigned char* srcRow;
	unsigned char* dstRow;
	unsigned char pixel, key;
	int width, height, fullWidth, fullHeight;
	int dstX, dstY;
	int srcX, srcY;

	key = pixels->format->colorkey;

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

		srcRow = ((unsigned char *)(pixels->pixels)) + (pixels->pitch * DIV(srcY, scale));
		dstRow = ((unsigned char *)(canvas->pixels)) + (canvas->pitch * dstY);

		if (x < (fullWidth >> 1)) {

			srcX = (fullWidth >> 1) - x;
			dstX = 0;

		} else {

			srcX = 0;
			dstX = x - (fullWidth >> 1);

		}

		while (srcX < width) {

			pixel = srcRow[DIV(srcX, scale)];
			if (pixel != key) dstRow[dstX] = pixel;

			srcX++;
			dstX++;

		}

		srcY++;
		dstY++;

	}

	if (SDL_MUSTLOCK(canvas)) SDL_UnlockSurface(canvas);

	return;

}

