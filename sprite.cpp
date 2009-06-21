
/*
 *
 * sprite.cpp
 *
 * Created on the 19th of March 2009 from parts of event.cpp and player.cpp
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


#include "level.h"
#include "palette.h"


Sprite::Sprite () {

	pixels = NULL;

	return;

}


Sprite::~Sprite () {

	if (pixels) SDL_FreeSurface(pixels);

	return;

}


void Sprite::clearPixels () {

	unsigned char *data;

	if (pixels) SDL_FreeSurface(pixels);

	data = new unsigned char[1];
	*data = SKEY;
	pixels = createSurface(data, 1, 1);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, SKEY);

	return;

}


void Sprite::setPixels (unsigned char *data, int width, int height) {

	if (pixels) SDL_FreeSurface(pixels);

	pixels = createSurface(data, width, height);
	SDL_SetColorKey(pixels, SDL_SRCCOLORKEY, SKEY);

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

	::restorePalette(pixels);

	return;

}


void Sprite::draw (int x, int y) {

	SDL_Rect dst;

	dst.x = x + xOffset;
	dst.y = y + yOffset;

	SDL_BlitSurface(pixels, NULL, screen, &dst);

	return;

}


Anim::Anim () {

	frame = 0;

	return;

}


Anim::~Anim () {

	return;

}


void Anim::setData (int amount, signed char x, signed char y) {

	frames = amount;
	xOffset = x << 2;
	yOffset = y;

	return;

}


void Anim::setFrame (int nextFrame, bool looping) {

	if (looping) frame = nextFrame % frames;
	else frame = (nextFrame >= frames)? frames - 1: nextFrame;

	return;

}


void Anim::setFrameData (Sprite *sprite, signed char x, signed char y) {

	sprites[frame] = sprite;
	xOffsets[frame] = x << 2;
	yOffsets[frame] = y;

	return;

}


int Anim::getWidth () {

	return sprites[frame]->getWidth();

}


int Anim::getHeight() {

	return sprites[frame]->getHeight();

}


void Anim::draw (int x, int y) {

	sprites[frame]->draw((x >> 10) + xOffsets[frame] - xOffset - (viewX >> 10),
		(y >> 10) + yOffsets[frame] - yOffset - (viewY >> 10));

	return;

}


void Anim::setPalette (SDL_Color *palette, int start, int amount) {

	sprites[frame]->setPalette(palette, 0, 256);

	return;

}


void Anim::flashPalette (int index) {

	sprites[frame]->flashPalette(index);

	return;

}


void Anim::restorePalette () {

	sprites[frame]->restorePalette();

	return;

}


