
/**
 *
 * @file guardians.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
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
 * @section Description
 * Provides the functions of guardian events.
 *
 */


#include "../level.h"
#include "../bullet.h"
#include "guardians.h"

#include "io/gfx/video.h"
#include "player/player.h"

#include "util.h"

#include <stdlib.h>


/**
 * Create guardian.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
Guardian::Guardian(unsigned char gX, unsigned char gY) : Event(gX, gY) {

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
 * Determine whether or not the guardian overlaps the given area.
 *
 * @param left The x-coordinate of the left of the area
 * @param top The y-coordinate of the top of the area
 * @param width The width of the area
 * @param height The height of the area
 *
 * @return Whether or not there is an overlap
 */
bool DeckGuardian::overlap (fixed left, fixed top, fixed width, fixed height) {

	if (stage == 0)
		return (x + F8 - F64 >= left) && (x - F64 < left + width) &&
			(y + F64 >= top) && (y + F32 < top + height);

	if (stage == 1)
		return (x + F32 >= left) && (x + F32 - F8 < left + width) &&
			(y + F64 >= top) && (y + F32 < top + height);

	if (stage == 2)
		return (x + F64 - F16 >= left) && (x + F32 - F8 < left + width) &&
			(y + F64 >= top) && (y + F32 < top + height);

	return false;

}


/**
 * Episode B guardian iteration.
 *
 * @param ticks Time
 * @param msps Ticks per step
 *
 * @return Remaining event
 */
Event* DeckGuardian::step (unsigned int ticks, int msps) {

	int count;


	set = prepareStep(ticks, msps);

	if (!set) return remove();


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

		if ((animType & ~1) == E_LFINISHANIM) {

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return remove();

		} else {

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

	Anim* anim;


	if (next) next->draw(ticks, change);


	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Draw the boss

	if (stage < 3) {

		// Draw unit

		anim = level->getAnim(29 + stage);

		if (ticks < flashTime) anim->flashPalette(0);

		if (stage == 0) anim->draw(getDrawX(change) - F64, getDrawY(change) + F32);
		else if (stage == 1) anim->draw(getDrawX(change) + F32 - F8 - F4, getDrawY(change) + F32);
		else anim->draw(getDrawX(change) + F8 - F64, getDrawY(change) + F32);

		if (ticks < flashTime) anim->restorePalette();

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
 * @param msps Ticks per step
 *
 * @return Remaining event
 */
Event* MedGuardian::step(unsigned int ticks, int msps) {

	Anim *anim = getAnim();

	fixed sin = fSin(ticks / 2);
	fixed cos = fCos(ticks / 2);

	set = prepareStep(ticks, msps);

	if (!set) return remove();


	if (level->getEventHits(gridX, gridY) >= set->strength / 2)
 		stage = 1;
	if (level->getEventHits(gridX, gridY) >= set->strength)
		stage = 2;

	// Stage 0: Move in an eight shape and fire the occasional shot

	if (stage == 0) {

		if (direction == 1) {

			// Lower right part of the eight
			animType = E_LEFTANIM;

			dx = TTOF(gridX) + (sin * 96) - x + ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos > 0) direction = 2;

		}

		if (direction == 2) {

			// Upper left part of the eight
			animType = E_LEFTANIM;

			dx = TTOF(gridX) - (sin * 96) - x - ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos < 0) direction = 3;

		}

		if (direction == 3) {

			// Lower left part of the eight
			animType = E_RIGHTANIM;

			dx = TTOF(gridX) - (sin * 96) - x - ITOF(96);
			dy = TTOF(gridY) - (cos * 64) - y;

			if (cos > 0) direction = 4;

		}

		if (direction == 4) {

			// Upper right part of the eight
			animType = E_RIGHTANIM;

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

			if ((set->bullet < 32) &&
					(level->getBullet(set->bullet)[B_SPRITE] != 0))
				level->bullets = new Bullet(
						x + anim->getAccessoryShootX(),
						y + anim->getAccessoryShootY(),
						set->bullet, (animType != E_LEFTANIM), ticks);

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
				dy = ITOF(-2);

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
					level->checkMaskUp(x + getWidth(), y))
				animType = (animType == E_LEFTANIM) ? E_RIGHTANIM : E_LEFTANIM;

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
			dx = (FTOI(x) % 2) ? ITOF(1) : ITOF(-1);
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

	dx = ((dx << 10) / msps);
	dy = ((dy << 10) / msps);
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;

	return this;

}


/**
 * Draw episode 1 guardian.
 *
 * @param ticks Time
 * @param change Time since last iteration
 */
void MedGuardian::draw(unsigned int ticks, int change) {

	Anim *anim;
	Anim *accessory;
	unsigned char frame;

	if (next) next->draw(ticks, change);

	fixed xChange = getDrawX(change);
	fixed yChange = getDrawY(change);


	frame = ticks / (set->animSpeed << 5);


	if (stage == 0)
		anim = getAnim();
	else
		anim = level->getAnim(set->anims[E_LFINISHANIM | (animType & 1)] & 0x7F);


	anim->setFrame(frame + gridX + gridY, true);

	if (ticks < flashTime) anim->flashPalette(0);

	anim->draw(xChange, yChange);

	if (ticks < flashTime) anim->restorePalette();

	accessory = anim->getAccessory();
	accessory->setFrame(frame + gridX + gridY, true);
	accessory->disableDefaultOffset();
	accessory->draw(xChange + (anim->getAccessoryX()), yChange + anim->getAccessoryY());

	return;

}

