
/*
 *
 * event.cpp
 *
 * 1st January 2006: Created events.c from parts of level.c
 * 3rd February 2009: Renamed events.c to events.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 11th February 2009: Created bullet.cpp from parts of events.cpp
 * 1st March 2009: Created bird.cpp from parts of events.cpp
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * 19th July 2009: Renamed events.cpp to event.cpp
 * 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
 * 2nd March 2010: Created bridge.cpp from parts of event.cpp and eventframe.cpp
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
 * Deals with events in ordinary levels.
 *
 */


#include "../level.h"
#include "event.h"

#include "io/sound.h"
#include "player/player.h"

#include "util.h"


Event::Event () {

	return;

}


Event::Event (unsigned char gX, unsigned char gY) {

	x = TTOF(gX);
	y = TTOF(gY + 1);
	dx = 0;
	dy = 0;

	next = level->events;
	gridX = gX;
	gridY = gY;
	flashTime = 0;

	onlyLAnimOffset = false;
	onlyRAnimOffset = false;
	noAnimOffset = false;

	switch (getProperty(E_BEHAVIOUR)) {

		case 2: // Walk from side to side
		case 4: // Walk from side to side and down hills

			animType = E_LEFTANIM;
			useRightAnimOffset();

			break;

		case 6: // Use the path from the level file
		case 7: // Flying snake behavior

			animType = E_LEFTANIM;
			dontUseAnimOffset();

			break;

		case 21: // Destructible block
		case 25: // Float up / Belt
		case 37: // Sucker tubes
		case 38: // Sucker tubes
		case 40: // Monochrome
		case 57: // Bubbles

			animType = 0;

			break;

		case 26: // Flip animation

			animType = E_RIGHTANIM;
			useLeftAnimOffset();

			break;

		default:

			animType = E_LEFTANIM;

			break;

	}

	return;

}


Event::~Event () {

	if (next) delete next;

	return;

}


Event* Event::remove () {

	Event *oldNext;

	oldNext = next;
	next = NULL;
	delete this;

	return oldNext;

}


Event * Event::getNext () {

	return next;

}


void Event::destroy (unsigned int ticks) {

	level->setEventTime(gridX, gridY, ticks + T_FINISH);

	animType = ((animType == E_RIGHTANIM) || (animType == E_RSHOOTANIM)) ?
			E_RFINISHANIM: E_LFINISHANIM;

	level->playSound(getProperty(E_SOUND));

	return;

}


bool Event::hit (LevelPlayer *source, unsigned int ticks) {

	int hitsRemaining;

	// Deal with bullet collisions

	// Check if event has already been destroyed
	if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM) ||
		(ticks < flashTime)) return false;

	hitsRemaining = level->hitEvent(gridX, gridY, source);

	// If the event cannot be hit, do not register hit
	if (hitsRemaining < 0) return false;

	// Check if the hit has destroyed the event
	if (hitsRemaining == 0) destroy(ticks);

	// The event has been hit, so it should flash
	flashTime = ticks + T_FLASH;

	// Register hit
	return true;

}


bool Event::isEnemy () {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	return set[E_HITSTOKILL] && (set[E_MODIFIER] == 0);

}


bool Event::isFrom (unsigned char gX, unsigned char gY) {

	return (gX == gridX) && (gY == gridY);

}


fixed Event::getWidth () {

	fixed width;

	if (!animType) return F32;

	if (getProperty(animType) <= 0) return 0;

	width = ITOF(level->getAnim(getProperty(animType))->getWidth());

	// Blank sprites for e.g. invisible springs
	if ((width == F1) && (getHeight() == F1)) return F32;

	return width;

}


fixed Event::getHeight () {

	if (!animType) return F32;

	if (getProperty(animType) <= 0) return 0;

	return ITOF(level->getAnim(getProperty(animType))->getHeight());

}


bool Event::overlap (fixed left, fixed top, fixed width, fixed height) {

	fixed offset = 0;
	if (getAnim(animType) && noAnimOffset)
		offset = getAnim(animType)->getOffset();

	return (x + getWidth() >= left) &&
		(x < left + width) &&
		(y + offset >= top) &&
		(y + offset - getHeight() < top + height);

}


signed char Event::getProperty (unsigned char property) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	if (set) return set[property];

	return 0;

}


Anim* Event::getAnim (unsigned char property) {

	switch (property) {

		case E_LEFTANIM:
		case E_RIGHTANIM:
		case E_LFINISHANIM:
		case E_RFINISHANIM:
		case E_LSHOOTANIM:
		case E_RSHOOTANIM:

			return level->getAnim(getProperty(property));

		default:

			return 0;

	}

}



