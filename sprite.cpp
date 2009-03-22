
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

#include "OpenJazz.h"

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


void Anim::setFrames (int newFrames) {

	frames = newFrames;

	return;

}


void Anim::setFrame (int newFrame, bool looping) {

	if (looping) frame = newFrame % frames;
	else frame = (newFrame >= frames)? frames - 1: newFrame;

	return;

}


void Anim::setData (Sprite *frameSprite, signed char frameY) {

	sprites[frame] = frameSprite;
	offsets[frame] = frameY;

	return;

}


int Anim::getOffset () {

	return offsets[frame];

}


int Anim::getWidth () {

	return sprites[frames - 1]->getWidth();

}


int Anim::getHeight() {

	return sprites[frames - 1]->getHeight();

}


void Anim::draw (int x, int y) {

	sprites[frame]->draw((x >> 10) - (viewX >> 10),
		(y >> 10) - (viewY >> 10) + offsets[frame]);

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


