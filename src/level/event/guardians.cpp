
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
#include "guardians.h"

#include "io/gfx/video.h"
#include "player/player.h"


DeckGuardian::DeckGuardian (unsigned char gX, unsigned char gY) {

	x = TTOF(gX);
	y = TTOF(gY + 1);
	dx = 0;
	dy = 0;

	next = level->events;
	gridX = gX;
	gridY = gY;
	flashTime = 0;

	stage = 0;

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


