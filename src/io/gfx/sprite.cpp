
/*
 *
 * sprite.cpp
 *
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 26th July 2009: Created anim.cpp from parts of sprite.cpp
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


#include "video.h"
#include "sprite.h"


Sprite::Sprite () {

	pixels = NULL;

	return;

}


Sprite::~Sprite () {

	if (pixels) SDL_FreeSurface(pixels);

	return;

}


void Sprite::clearPixels () {

	unsigned char data;

	if (pixels) SDL_FreeSurface(pixels);

	data = SKEY;
	pixels = createSurface(&data, 1, 1);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, SKEY);

	return;

}


void Sprite::setPixels (unsigned char *data, int width, int height) {

	unsigned char *sorted;

	if (pixels) SDL_FreeSurface(pixels);

	sorted = sortPixels(data, width * height);

	pixels = createSurface(sorted, width, height);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, SKEY);

	delete[] sorted;

	return;

}


int Sprite::getWidth () {

	return pixels->w;

}


int Sprite::getHeight() {

	return pixels->h;

}


void Sprite::setPalette (SDL_Color *palette, int start, int amount) {

	SDL_SetPalette(pixels, SDL_LOGPAL, palette + start, start, amount);

	return;

}


void Sprite::flashPalette (int index) {

	SDL_Color palette[256];
	int count;

	// Map the whole palette to one index

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = index;

	SDL_SetPalette(pixels, SDL_LOGPAL, palette, 0, 256);

	return;

}


void Sprite::restorePalette () {

	video.restoreSurfacePalette(pixels);

	return;

}


void Sprite::draw (int x, int y) {

	SDL_Rect dst;

	dst.x = x + xOffset;
	dst.y = y + yOffset;

	SDL_BlitSurface(pixels, NULL, canvas, &dst);

	return;

}


