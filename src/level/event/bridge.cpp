
/*
 *
 * guardians.cpp
 *
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
 * Provides the functions of bridge events.
 *
 */


#include "../level.h"
#include "event.h"

#include "player/player.h"


Bridge::Bridge (unsigned char gX, unsigned char gY, Event *nextEvent) {

	x = TTOF(gX);
	y = TTOF(gY + 1);
	dx = 0;
	dy = 0;

	next = nextEvent;
	gridX = gX;
	gridY = gY;
	flashTime = 0;

	animType = E_LEFTANIM;
	x -= F2;
	y += ITOF(getProperty(E_YAXIS)) - F40;

	// dx and dy used to store leftmost and rightmost player on bridge
	// Start with minimum values
	dx = getProperty(E_MULTIPURPOSE) * F8;
	dy = 0;

	return;

}


bool Bridge::step (unsigned int ticks, int msps) {

	signed char *set;
	int count;
	fixed offset;


	set = prepareStep(ticks, msps);

	if (!set) return true;


	// Gradually stop the bridge sagging
	if (dx < set[E_MULTIPURPOSE] * F8) dx += 320 * msps;
	if (dx > set[E_MULTIPURPOSE] * F8) dx = set[E_MULTIPURPOSE] * F8;
	if (dy > 0) dy -= 320 * msps;
	if (dy < 0) dy = 0;


	for (count = 0; count < nPlayers; count++) {

		offset = players[count].getX() + PXO_MID;

		if (players[count].overlap(x, y, set[E_MULTIPURPOSE] * F8, F8) &&
			!level->checkMaskDown(offset, y - F32)) {

			// Player is on the bridge

			players[count].setEvent(gridX, gridY);

			offset -= x;

			if (offset < dx) dx = offset;

			if ((offset > dy) && (offset < set[E_MULTIPURPOSE] * F8)) dy = offset;

			if (offset < set[E_MULTIPURPOSE] * F4)
				players[count].setPosition(players[count].getX(), y + (offset >> 3) - F8);
			else
				players[count].setPosition(players[count].getX(), y + (set[E_MULTIPURPOSE] * F1) - (offset >> 3) - F8);

		} else players[count].clearEvent(gridX, gridY);

	}


	return false;

}


void Bridge::draw (unsigned int ticks, int change) {

	Anim *anim;
	signed char *set;
	int count;
	fixed bridgeLength, dipA, dipB;


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Check if the event has anything to draw
	if (!animType || (set[animType] < 0)) return;


	if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))
		frame = (ticks + T_FINISH - level->getEventTime(gridX, gridY)) / 40;
	else if (set[E_ANIMSP])
		frame = ticks / (set[E_ANIMSP] * 40);
	else
		frame = ticks / 20;

	anim = level->getAnim(set[animType]);
	anim->setFrame(frame + gridX + gridY, true);


	// Draw the bridge

	bridgeLength = set[E_MULTIPURPOSE] * F8;

	if (dy >= dx) {

		dipA = (dx <= (bridgeLength >> 1)) ? dx >> 3: (bridgeLength - dx) >> 3;
		dipB = (dy <= (bridgeLength >> 1)) ? dy >> 3: (bridgeLength - dy) >> 3;

		for (count = 0; count < bridgeLength; count += F8) {

			if (count < dx)
				anim->draw(x + count, y + (count * dipA / dx));
			else if (count < dy)
				anim->draw(x + count, y + dipA + ((count - dx) * (dipB - dipA) / (dy - dx)));
			else
				anim->draw(x + count, y + ((bridgeLength - count) * dipB / (bridgeLength - dy)));

		}

	} else {

		// No players on the bridge, de-sagging in progress

		dipA = (dx + dy) >> 1;
		dipB = (dy < bridgeLength - dx) ? dy >> 3: (bridgeLength - dx) >> 3;

		for (count = 0; count < bridgeLength; count += F8) {

			if (count < dipA)
				anim->draw(x + count, y + (count * dipB / dipA));
			else
				anim->draw(x + count, y + ((bridgeLength - count) * dipB / (bridgeLength - dipA)));

		}

	}


	return;

}


