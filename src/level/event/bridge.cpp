
/*
 *
 * bridge.cpp
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


Bridge::Bridge (unsigned char gX, unsigned char gY) {

	signed char* set;

	set = level->getEvent(gX, gY);

	x = TTOF(gX);
	y = TTOF(gY) + ITOF(set[E_YAXIS]) - F8 - F1;
	dx = 0;
	dy = 0;

	next = level->events;
	gridX = gX;
	gridY = gY;
	animType = E_LEFTANIM;
	flashTime = 0;

	// Bridges should ignore the default yOffsets
	getAnim(E_LEFTANIM)->disableYOffset();
	getAnim(E_RIGHTANIM)->disableYOffset();

	// leftDipX and rightDipX used to store leftmost and rightmost player on bridge
	// Start with minimum values
	leftDipX = set[E_MULTIPURPOSE] * set[E_BRIDGELENGTH] * F4;
	rightDipX = 0;

	return;

}


Event* Bridge::step (unsigned int ticks, int msps) {

	signed char* set;
	int count;
	fixed bridgeLength, playerDipX, playerDipY;


	set = prepareStep(ticks, msps);

	if (!set) return remove();


	bridgeLength = set[E_MULTIPURPOSE] * set[E_BRIDGELENGTH] * F4;


	// Gradually stop the bridge sagging
	if (leftDipX < bridgeLength) leftDipX += 320 * msps;
	if (leftDipX > bridgeLength) leftDipX = bridgeLength;
	if (rightDipX > 0) rightDipX -= 320 * msps;
	if (rightDipX < 0) rightDipX = 0;


	for (count = 0; count < nPlayers; count++) {

		playerDipX = players[count].getX() + PXO_MID - x;

		if (playerDipX < bridgeLength >> 1) playerDipY = playerDipX >> 3;
		else playerDipY = (bridgeLength - playerDipX) >> 3;

		if (players[count].overlap(x, y + playerDipY - F4, bridgeLength, F8) &&
			!level->checkMaskDown(x + playerDipX, y + playerDipY - F32)) {

			// Player is on the bridge

			players[count].setEvent(gridX, gridY);

			if (playerDipX < leftDipX) leftDipX = playerDipX;

			if (playerDipX > rightDipX) rightDipX = playerDipX;

			players[count].setPosition(players[count].getX(), y + playerDipY);

		} else players[count].clearEvent(gridX, gridY);

	}


	return this;

}


void Bridge::draw (unsigned int ticks, int change) {

	Anim *anim;
	signed char *set;
	int count;
	fixed bridgeLength, leftDipY, rightDipY;


	if (next) next->draw(ticks, change);


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Check if the event has anything to draw
	if (!animType || (set[animType] < 0)) return;


	if (set[E_ANIMSP]) frame = ticks / (set[E_ANIMSP] * 40);
	else frame = ticks / 20;

	anim = level->getAnim(set[animType]);
	anim->setFrame(frame + gridX + gridY, true);


	// Draw the bridge

	bridgeLength = set[E_MULTIPURPOSE] * set[E_BRIDGELENGTH] * F4;

	if (rightDipX >= leftDipX) {

		leftDipY = (leftDipX <= (bridgeLength >> 1)) ? leftDipX >> 3: (bridgeLength - leftDipX) >> 3;
		rightDipY = (rightDipX <= (bridgeLength >> 1)) ? rightDipX >> 3: (bridgeLength - rightDipX) >> 3;

		for (count = 0; count < bridgeLength; count += F4 * set[E_BRIDGELENGTH]) {

			if (count < leftDipX)
				anim->draw(getDrawX(change) + count, getDrawY(change) + (count * leftDipY / leftDipX));
			else if (count < dy)
				anim->draw(getDrawX(change) + count, getDrawY(change) + leftDipY + ((count - leftDipX) * (rightDipY - leftDipY) / (rightDipX - leftDipX)));
			else
				anim->draw(getDrawX(change) + count, getDrawY(change) + ((bridgeLength - count) * rightDipY / (bridgeLength - rightDipX)));

		}

	} else {

		// No players on the bridge, de-sagging in progress

		// Midpoint
		leftDipY = (leftDipX + rightDipX) >> 1;

		// Dip
		rightDipY = (rightDipX < bridgeLength - leftDipX) ? rightDipX >> 3: (bridgeLength - leftDipX) >> 3;

		for (count = 0; count < bridgeLength; count += F4 * set[E_BRIDGELENGTH]) {

			if (count < leftDipY)
				anim->draw(getDrawX(change) + count, getDrawY(change) + (count * rightDipY / leftDipY));
			else
				anim->draw(getDrawX(change) + count, getDrawY(change) + ((bridgeLength - count) * rightDipY / (bridgeLength - leftDipY)));

		}

	}


	return;

}


