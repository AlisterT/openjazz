
/*
 *
 * guardians.cpp
 *
 * 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
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

/*
 * Provides the functions of guardian events.
 *
 */


#include "../level.h"
#include "../bullet.h"
#include "guardians.h"

#include "io/gfx/video.h"
#include "player/player.h"

#include "util.h"


Guardian::Guardian(unsigned char gX, unsigned char gY) {

	x = TTOF(gX);
	y = TTOF(gY + 1);
	dx = 0;
	dy = 0;

	next = level->getEvents();
	gridX = gX;
	gridY = gY;
	flashTime = 0;

	stage = 0;

	return;

}


DeckGuardian::DeckGuardian (unsigned char gX, unsigned char gY) : Guardian(gX, gY) {

	return;

}


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


Event* DeckGuardian::step (unsigned int ticks, int msps) {

	signed char* set;
	int count;


	set = prepareStep(ticks, msps);

	if (!set) return remove();


	// Handle behaviour

	count = level->getEventHits(gridX, gridY);

	if (count < 8) stage = 0;
	else if (count < 16) stage = 1;
	else if (count < 24) stage = 2;
	else stage = 3;


	// If the event has been destroyed, play its finishing animation and set its
	// reaction time
	if (set[E_HITSTOKILL] &&
		(level->getEventHits(gridX, gridY) >= set[E_HITSTOKILL]) &&
		(animType != E_LFINISHANIM) && (animType != E_RFINISHANIM)) {

		destroy(ticks);

	}


	// If the reaction time has expired
	if (level->getEventTime(gridX, gridY) &&
		(ticks > level->getEventTime(gridX, gridY))) {

		if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM)) {

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return remove();

		} else {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	return this;

}


void DeckGuardian::draw (unsigned int ticks, int change) {

	Anim* anim;
	signed char* set;


	if (next) next->draw(ticks, change);


	// Get the event properties
	set = level->getEvent(gridX, gridY);

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


MedGuardian::MedGuardian(unsigned char gX, unsigned char gY) : Guardian(gX, gY) {

	animType = E_LEFTANIM;
	stage = 0;
	direction = 1;
	shoot = false;

	return;

}


/*bool MedGuardian::overlap(fixed left, fixed top, fixed width, fixed height) {

	return false;

}*/


Event* MedGuardian::step(unsigned int ticks, int msps) {

	Anim *anim = getAnim(animType);

	fixed sin = fSin(ticks / 2);
	fixed cos = fCos(ticks / 2);

	if (level->getEventHits(gridX, gridY) == getProperty(E_HITSTOKILL) / 2)
 		stage = 1;

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
		if ((ticks % (getProperty(E_BULLETSP) * 25) >
				(unsigned int)(getProperty(E_BULLETSP) * 25) - T_SHOOT)) {

			level->setEventTime(gridX, gridY, ticks + T_SHOOT);
			shoot = true;

		}

		// Shoot if there is a shot
		if (level->getEventTime(gridX, gridY) &&
				(ticks > level->getEventTime(gridX, gridY)) &&
				shoot) {

			if ((getProperty(E_BULLET) < 32) &&
					(level->getBullet(getProperty(E_BULLET))[B_SPRITE] != 0))
				level->bullets = new Bullet(
						x + anim->getAccessoryShootX(),
						y + anim->getAccessoryShootY(),
						getProperty(E_BULLET), (animType != E_LEFTANIM), ticks);

			shoot = false;

		}

	}

	// Stage 1: Hop back and forth destroying the bottom row of tiles

	if (stage == 1) {

		if (direction < 5) {

			// Move up or down towards the starting position for hopping
			if (y > TTOF(gridY) + ITOF(48))
				direction = 5;
			else
				direction = 6;

		}

		// Move up to the correct height
		if (direction == 5) {

			if (y > TTOF(gridY) + ITOF(40)) {

				dx = 0;
				dy = ITOF(-2);

			}
			else direction = 7;

		}

		// Move down to the correct height
		if (direction == 6) {

			if (y < TTOF(gridY) + ITOF(40)) {

				dx = 0;
				dy = ITOF(2);

			} else direction = 7;

		}

		// Wait until the cosinus is zero.
		if (direction == 7) {

			dx = 0;
			dy = 0;

			if (cos > 0 && cos < 200) direction = 8;

		}

		// Start hopping
		if (direction == 8) {
			if (level->checkMaskUp(x, y) ||
					level->checkMaskUp(x + getWidth(), y))
				animType = (animType == E_LEFTANIM) ? E_RIGHTANIM : E_LEFTANIM;

			dy = TTOF(gridY) + ITOF(40) - abs(cos * 96) - y;

			dx = -abs(cos * 6);
			if (animType == E_RIGHTANIM)
				dx *= -1;
		}


		// Stand still and shake
		/*if (direction == 6) {

			// Set a timer
			if (level->getEventTime(gridX, gridY) &&
					(ticks > level->getEventTime(gridX, gridY))) {

				dx = 0;
				dy = 0;

			}
			else
				level->setEventTime(gridX, gridY, ticks + 2000);
		}*/

	}

	dx = ((dx << 10) / msps);
	dy = ((dy << 10) / msps);
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;

	return this;

}


void MedGuardian::draw(unsigned int ticks, int change) {

	Anim *anim;
	Anim *accessory;

	if (next) next->draw(ticks, change);

	fixed xChange = getDrawX(change);
	fixed yChange = getDrawY(change);


	if (getProperty(E_ANIMSP))
		frame = ticks / (getProperty(E_ANIMSP) * 40);
	else
		frame = ticks / 20;


	if (stage == 0)
		anim = getAnim(animType);
	else
		anim = getAnim(animType == E_LEFTANIM ? E_LFINISHANIM : E_RFINISHANIM);


	anim->setFrame(frame + gridX + gridY, true);
	anim->draw(xChange, yChange);

	accessory = anim->getAccessory();
	accessory->setFrame(frame + gridX + gridY, true);
	accessory->disableDefaultOffset();
	accessory->draw(xChange + (anim->getAccessoryX()), yChange + anim->getAccessoryY());

	return;

}

