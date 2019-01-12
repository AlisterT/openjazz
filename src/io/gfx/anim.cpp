
/**
 *
 * @file anim.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * - 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * - 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * - 26th July 2009: Created anim.cpp from parts of sprite.cpp
 *
 * @par Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#include "anim.h"
#include "sprite.h"

#include "jj1level/jj1level.h"


/**
 * Create empty animation.
 */
Anim::Anim () {

	sprites = new Sprite *[19];
	xOffsets = new signed char[19];
	yOffsets = new signed char[19];

	frame = 0;
	yOffset = 0;

	return;

}


/**
 * Delete animation.
 */
Anim::~Anim () {

	delete[] sprites;
	delete[] xOffsets;
	delete[] yOffsets;

	return;

}


/**
 * Set overall animation data.
 *
 * @param length Number of frames
 * @param sX Bullet generation x-coordinate
 * @param sY Bullet generation y-coordinate
 * @param aX Accessory animation x-coordinate
 * @param aY Accessory animation y-coordinate
 * @param a Accessory animation index
 * @param y Vertical offset
 */
void Anim::setData (int length, signed char sX, signed char sY, signed char aX, signed char aY, unsigned char a, signed char y) {

	if (length > 19) {

		delete[] sprites;
		delete[] xOffsets;
		delete[] yOffsets;

		sprites = new Sprite *[length];
		xOffsets = new signed char[length];
		yOffsets = new signed char[length];

	}

	frames = length;
	shootX = sX;
	shootY = sY;
	accessoryX = aX;
	accessoryY = aY;
	accessory = a;
	yOffset = y;

	return;

}


/**
 * Set current frame.
 *
 * @param nextFrame The frame to use
 * @param looping Whether the animation should stop at the end or loop
 */
void Anim::setFrame (int nextFrame, bool looping) {

	if (looping) frame = nextFrame % frames;
	else frame = (nextFrame >= frames)? frames - 1: nextFrame;

	return;

}


/**
 * Set the data for the current frame.
 *
 * @param sprite Sprite to use
 * @param x Horizontal offset
 * @param y Vertical offset
 */
void Anim::setFrameData (Sprite *sprite, signed char x, signed char y) {

	sprites[frame] = sprite;
	xOffsets[frame] = x;
	yOffsets[frame] = y;

	return;

}


/**
 * Determine the width of the current frame.
 *
 * @return The width of the current frame
 */
int Anim::getWidth () {

	return sprites[frame]->getWidth();

}


/**
 * Determine the height of the current frame.
 *
 * @return The height of the current frame
 */
int Anim::getHeight () {

	return sprites[frame]->getHeight();

}


/**
 * Determine the length of the animation.
 *
 * @return The length of the animation
 */
int Anim::getLength () {

	return frames;

}


/**
 * Determine the bullet generation x-coordinate of the current frame.
 *
 * @return The bullet generation x-coordinate
 */
fixed Anim::getShootX () {

	return ITOF(shootX + xOffsets[frame]) << 2;

}


/**
 * Determine the bullet generation y-coordinate of the current frame.
 *
 * @return The bullet generation y-coordinate
 */
fixed Anim::getShootY () {

	return ITOF(shootY + yOffsets[frame]);

}


/**
 * Determine the accessory bullet generation x-coordinate of the current frame.
 *
 * @return The accessory bullet generation x-coordinate
 */
fixed Anim::getAccessoryShootX () {

	return ITOF(shootX + accessoryX + xOffsets[frame]) << 2;

}


/**
 * Determine the accessory bullet generation y-coordinate of the current frame.
 *
 * @return The accessory bullet generation y-coordinate
 */
fixed Anim::getAccessoryShootY () {

	return ITOF(shootY + accessoryY + yOffsets[frame]);

}


/**
 * Determine the vertical offset.
 *
 * @return The vertical offset
 */
fixed Anim::getOffset () {

	return -ITOF(yOffset);

}


/**
 * Determine the horizontal offset of the current frame.
 *
 * @return The horizontal offset
 */
fixed Anim::getXOffset () {

	return ITOF(sprites[frame]->getXOffset() + (xOffsets[frame] << 2) + 1);

}


/**
 * Determine the vertical offset of the current frame.
 *
 * @return The vertical offset
 */
fixed Anim::getYOffset () {

	return ITOF(sprites[frame]->getYOffset() + yOffsets[frame] + 1);

}


/**
 * Draw current frame.
 *
 * @param x X-coordinate at which to draw
 * @param y Y-coordinate at which to draw
 * @param accessories Number of accessory animations to draw
 */
void Anim::draw (fixed x, fixed y, int accessories) {

	Anim* anim;

	sprites[frame]->draw(
		FTOI(x) + (xOffsets[frame] << 2),
		FTOI(y) + yOffsets[frame] - yOffset);


	if (accessories && accessory) {

		anim = level->getAnim(accessory);
		anim->setFrame(frame, true);
		anim->draw(
			x + ITOF(accessoryX << 2),
			y + ITOF(accessoryY - yOffset) - anim->getOffset(),
			accessories - 1);

	}


	return;

}


/**
 * Draw current frame scaled.
 *
 * @param x X-coordinate at which to draw
 * @param y Y-coordinate at which to draw
 * @param scale Scaling factor
 */
void Anim::drawScaled (fixed x, fixed y, fixed scale) {

	// Used to draw bonus level player, so no offset
	sprites[frame]->drawScaled(FTOI(x), FTOI(y), scale);

	return;

}


/**
 * Set the current frame's palette.
 *
 * @param palette The new palette to use
 * @param start The first entry to use
 * @param amount The number of entries to use
 */
void Anim::setPalette (SDL_Color *palette, int start, int amount) {

	sprites[frame]->setPalette(palette, start, amount);

	return;

}


/**
 * Turn the whole of the current frame a single colour.
 *
 * @param index The index of the colour to use
 */
void Anim::flashPalette (int index) {

	sprites[frame]->flashPalette(index);

	return;

}


/**
 * Restore the current frame's original palette.
 */
void Anim::restorePalette () {

	sprites[frame]->restorePalette();

	return;

}


