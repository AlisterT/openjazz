
/**
 *
 * @file jj1guardians.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp
 * - 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * - 19th July 2009: Renamed events.cpp to event.cpp
 * - 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
 * - 1st August 2012: Renamed guardians.cpp to jj1guardians.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Provides the functions of guardian events.
 *
 */


#include "../jj1bullet.h"
#include "../jj1level.h"
#include "jj1guardians.h"

#include "io/gfx/video.h"
#include "util.h"

#include <stdlib.h>


/**
 * Create guardian.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
Guardian::Guardian(unsigned char gX, unsigned char gY) : JJ1Event(gX, gY) {

	setAnimType(E_LEFTANIM);

	stage = 0;

	return;

}


/**
 * Create episode B guardian.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
DeckGuardian::DeckGuardian (unsigned char gX, unsigned char gY) : Guardian(gX, gY) {

	return;

}


/**
 * Episode B guardian iteration.
 *
 * @param ticks Time
 *
 * @return Remaining event
 */
JJ1Event* DeckGuardian::step (unsigned int ticks) {

	int count;


	set = prepareStep(ticks);

	if (!set) return remove(false);


	count = level->getEventHits(gridX, gridY);

	if (count < 8) stage = 0;
	else if (count < 16) stage = 1;
	else if (count < 24) stage = 2;
	else stage = 3;


	// If the event has been destroyed, play its finishing animation and set its
	// reaction time
	if (set->strength &&
		(level->getEventHits(gridX, gridY) >= set->strength) &&
		((animType & ~1) != E_LFINISHANIM)) {

		destroy(ticks);

	}


	// If the reaction time has expired
	if (level->getEventTime(gridX, gridY) &&
		(ticks > level->getEventTime(gridX, gridY))) {

		if ((animType & ~1) != E_LFINISHANIM) {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	return this;

}


/**
 * Draw episode B guardian.
 *
 * @param ticks Time
 * @param change Time since last iteration
 */
void DeckGuardian::draw (unsigned int ticks, int change) {

	Anim* unitAnim;


	if (next) next->draw(ticks, change);


	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Draw the boss

	if (stage < 3) {

		// Draw unit

		unitAnim = level->getAnim(29 + stage);

		if (stage == 0) {

			width = F8;
			drawnX = x - F64;

		} else if (stage == 1) {

			width = F8;
			drawnX = x + F32 - F8;

		} else if (stage == 2) {

			width = F64 + F32;
			drawnX = x - F64;

		}

		drawnY = y + F32;
		height = F32;

		if (ticks < flashTime) unitAnim->flashPalette(0);

		if (stage == 0) unitAnim->draw(getDrawX(change) - F64, getDrawY(change) + F32);
		else if (stage == 1) unitAnim->draw(getDrawX(change) + F32 - F8 - F4, getDrawY(change) + F32);
		else unitAnim->draw(getDrawX(change) + F8 - F64, getDrawY(change) + F32);

		if (ticks < flashTime) unitAnim->restorePalette();

	}


	return;

}


/**
 * Create episode 1 guardian.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
MedGuardian::MedGuardian(unsigned char gX, unsigned char gY) : Guardian(gX, gY) {

	direction = 1;
	shoot = false;

	return;

}


/**
 * Episode 1 guardian iteration.
 *
 * @param ticks Time
 *
 * @return Remaining event
 */
JJ1Event* MedGuardian::step(unsigned int ticks) {

	fixed sin = fSin(ticks / 2);
	fixed cos = fCos(ticks / 2);

	set = prepareStep(ticks);

	if (!set) return remove(false);


	if (level->getEventHits(gridX, gridY) >= set->strength / 2)
 		stage = 1;
	if (level->getEventHits(gridX, gridY) >= set->strength)
		stage = 2;

	// Stage 0: Move in an eight shape and fire the occasional shot

	if (stage == 0) {

		if (direction == 1) {

			// Lower right part of the eight
			setAnimType(E_LEFTANIM);

			dx = TTOF(gridX) + (sin * 96) - x + ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos > 0) direction = 2;

		}

		if (direction == 2) {

			// Upper left part of the eight
			setAnimType(E_LEFTANIM);

			dx = TTOF(gridX) - (sin * 96) - x - ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos < 0) direction = 3;

		}

		if (direction == 3) {

			// Lower left part of the eight
			setAnimType(E_RIGHTANIM);

			dx = TTOF(gridX) - (sin * 96) - x - ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos > 0) direction = 4;

		}

		if (direction == 4) {

			// Upper right part of the eight
			setAnimType(E_RIGHTANIM);

			dx = TTOF(gridX) + (sin * 96) - x + ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos < 0) direction = 1;

		}

		// Decide if there should be a shot
		if ((ticks % (set->bulletPeriod * 25) >
				(unsigned int)(set->bulletPeriod * 25) - 300)) {

			level->setEventTime(gridX, gridY, ticks + 300);
			shoot = true;

		}

		// Shoot if there is a shot
		if (level->getEventTime(gridX, gridY) &&
				(ticks > level->getEventTime(gridX, gridY)) &&
				shoot) {

			if (set->bullet < 32)
				level->createBullet(NULL,
					gridX,
					gridY,
					x + anim->getAccessoryShootX(),
					y + anim->getAccessoryShootY(),
					set->bullet,
					(animType != E_LEFTANIM),
					ticks);

			shoot = false;

		}

	}

	// Stage 1: Hop back and forth destroying the bottom row of tiles

	if (stage == 1) {

		fixed startPos = TTOF(gridY) + ITOF(40);

		if (direction < 5) {

			// Move up or down towards the starting position for hopping
			direction = (y > startPos) ? 5 : 6;

		}

		// Move up to the correct height
		if (direction == 5) {

			if (y > startPos) {

				dx = 0;
				dy = -ITOF(2);

			}
			else direction = 7;

		}

		// Move down to the correct height
		if (direction == 6) {

			if (y < startPos) {

				dx = 0;
				dy = ITOF(2);

			} else direction = 7;

		}

		// Cosinus should be near zero before we start hopping.
		if (direction == 7) {

			dx = 0;
			dy = 0;

			if (cos > -100 && cos < 100) direction = 8;

		}

		// Start hopping
		if (direction == 8) {

			if (level->checkMaskUp(x, y) ||
					level->checkMaskUp(x + width, y))
				setAnimType((animType == E_LEFTANIM) ? E_RIGHTANIM : E_LEFTANIM);

			dy = startPos - abs(cos * 96) - y;
			dx = abs(cos * 6);

			if (animType == E_LEFTANIM)
				dx *= -1;

			if (cos < 0 &&
					level->checkMaskDown(x + ITOF(anim->getWidth() / 2), y + TTOF(1)))
				direction = 9;

		}

		// Destroy the block underneath
		if (direction == 9) {

			// Shake a bit
			dx = (FTOI(x) % 2) ? ITOF(1) : -ITOF(1);
			dy = 0;

			// Remove the tile
			if (cos > 0 && cos < 100) {

				level->setTile(
						FTOT(x + ITOF((anim->getWidth() / 2))),
						FTOT(y) + 1,
						set->magnitude);

				direction = 8;

			}

		}

	}

	// Stage 2: End of behavior

	if (stage == 2) {

		dx = 0;
		dy = ITOF(4);

	}

	x += dx;
	y += dy;
	dx = dx << 6;
	dy = dy << 6;

	return this;

}


/**
 * Draw episode 1 guardian.
 *
 * @param ticks Time
 * @param change Time since last iteration
 */
void MedGuardian::draw(unsigned int ticks, int change) {

	Anim *stageAnim;
	unsigned char frame;

	if (next) next->draw(ticks, change);

	fixed xChange = getDrawX(change);
	fixed yChange = getDrawY(change);


	frame = ticks / (set->animSpeed << 5);


	if (stage == 0)
		stageAnim = anim;
	else
		stageAnim = level->getAnim(set->anims[E_LFINISHANIM | (animType & 1)] & 0x7F);


	stageAnim->setFrame(frame + gridX + gridY, true);

	if (ticks < flashTime) stageAnim->flashPalette(0);

	drawnX = x + anim->getXOffset();
	drawnY = y + anim->getYOffset() + stageAnim->getOffset();

	stageAnim->draw(xChange, yChange);

	if (ticks < flashTime) stageAnim->restorePalette();


	return;

}

