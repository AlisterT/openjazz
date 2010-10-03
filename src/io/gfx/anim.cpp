
/**
 *
 * @file anim.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 26th July 2009: Created anim.cpp from parts of sprite.cpp
 *
 * @section Licence
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


#include "anim.h"
#include "sprite.h"

#include "level/level.h"


Anim::Anim () {

	sprites = new Sprite *[19];
	xOffsets = new signed char[19];
	yOffsets = new signed char[19];

	frame = 0;
	yOffset = 0;
	ignoreDefaultYOffset = false;

	return;

}


Anim::~Anim () {

	delete[] sprites;
	delete[] xOffsets;
	delete[] yOffsets;

	return;

}


void Anim::setData (int amount, signed char sX, signed char sY, signed char aX, signed char aY, unsigned char a, signed char y) {

	if (amount > 19) {

		delete[] sprites;
		delete[] xOffsets;
		delete[] yOffsets;

		sprites = new Sprite *[amount];
		xOffsets = new signed char[amount];
		yOffsets = new signed char[amount];

	}

	frames = amount;
	shootX = sX;
	shootY = sY;
	accessoryX = aX;
	accessoryY = aY;
	accessory = a;
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
	xOffsets[frame] = x;
	yOffsets[frame] = y;

	return;

}


int Anim::getWidth () {

	return sprites[frame]->getWidth();

}


int Anim::getHeight () {

	return sprites[frame]->getHeight();

}


fixed Anim::getShootX () {

	return ITOF(shootX + (xOffsets[frame] << 2));

}


fixed Anim::getShootY () {

	return ITOF(shootY + yOffsets[frame] - yOffset);

}


fixed Anim::getAccessoryX () {

	return ITOF(accessoryX << 2);

}


fixed Anim::getAccessoryY () {

	return ITOF(accessoryY - yOffset);

}


fixed Anim::getAccessoryShootX () {

	return ITOF(shootX + (accessoryX << 2) + xOffsets[frame]);

}


fixed Anim::getAccessoryShootY () {

	return ITOF(shootY + accessoryY + yOffsets[frame] - yOffset);

}


fixed Anim::getOffset () {

	if (!ignoreDefaultYOffset && yOffset == 0)
		return ITOF(yOffset) + TTOF(1);

	return ITOF(yOffset);

}


Anim* Anim::getAccessory() {

	return level->getAnim(accessory);

}


void Anim::draw (fixed x, fixed y) {

	// In case yOffset is zero, and the ignore default offset flag is set,
	// draw the animation without any offset.

	if (ignoreDefaultYOffset && yOffset == 0)
		sprites[frame]->draw(FTOI(x) + (xOffsets[frame] << 2) + 1,
				FTOI(y) + yOffsets[frame] + 1);

	// In case yOffset is zero, most animations need a default offset
	// of 1 tile (32 pixels).

	else if (yOffset == 0)
		sprites[frame]->draw(FTOI(x) + (xOffsets[frame] << 2) + 1,
				FTOI(y) + yOffsets[frame] - TTOI(1) + 2);

	// In all other cases drawing with the Y offset will do.

	else
		sprites[frame]->draw(FTOI(x) + (xOffsets[frame] << 2) + 1,
				FTOI(y) + yOffsets[frame] - yOffset + 1);

	return;

}


void Anim::drawScaled (fixed x, fixed y, fixed scale) {

	// Used to draw bonus level player, so no offset
	sprites[frame]->drawScaled(FTOI(x), FTOI(y), scale);

	return;

}


void Anim::disableDefaultOffset() {

	ignoreDefaultYOffset = true;

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


