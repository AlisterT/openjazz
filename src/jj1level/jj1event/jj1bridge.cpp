
/**
 *
 * @file jj1bridge.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp
 * - 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * - 19th July 2009: Renamed events.cpp to event.cpp
 * - 2nd March 2010: Created bridge.cpp from parts of event.cpp and eventframe.cpp
 * - 1st August 2012: Renamed bridge.cpp to jj1bridge.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Provides the functions of bridge events.
 *
 */


#include "../jj1level.h"
#include "../jj1levelplayer/jj1levelplayer.h"
#include "jj1event.h"


/**
 * Create bridge.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
JJ1Bridge::JJ1Bridge (unsigned char gX, unsigned char gY) : JJ1Event(gX, gY) {

	y = TTOF(gY) + ITOF(set->multiB);

	setAnimType(E_LEFTANIM);

	// Bridges should ignore the default yOffsets
	noAnimOffset = true;

	// leftDipX and rightDipX used to store leftmost and rightmost player on bridge
	// Start with minimum values
	leftDipX = set->multiA * set->pieceSize * F4;
	rightDipX = 0;

	return;

}


/**
 * Bridge iteration.
 *
 * @param ticks Time
 *
 * @return Remaining event
 */
JJ1Event* JJ1Bridge::step (unsigned int ticks) {

	JJ1LevelPlayer* levelPlayer;
	int count;
	fixed bridgeLength, playerDipX, playerDipY;


	set = prepareStep(ticks);

	if (!set) return remove(false);


	bridgeLength = set->multiA * set->pieceSize * F4;


	// Gradually stop the bridge sagging
	if (leftDipX < bridgeLength) leftDipX += 5120;
	if (leftDipX > bridgeLength) leftDipX = bridgeLength;
	if (rightDipX > 0) rightDipX -= 5120;
	if (rightDipX < 0) rightDipX = 0;


	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getJJ1LevelPlayer();

		playerDipX = levelPlayer->getX() + PXO_MID - x;

		if (playerDipX < bridgeLength >> 1) playerDipY = playerDipX >> 3;
		else playerDipY = (bridgeLength - playerDipX) >> 3;

		if (levelPlayer->overlap(x, y - F8 + playerDipY - F4, bridgeLength, F8) &&
			!level->checkMaskDown(x + playerDipX, y - F8 + playerDipY - F32)) {

			// Player is on the bridge

			if (playerDipX < leftDipX) leftDipX = playerDipX;

			if (playerDipX > rightDipX) rightDipX = playerDipX;

			levelPlayer->setPlatform(gridX, gridY, 0, y - F8 - F1 + playerDipY);

		} else levelPlayer->clearEvent(gridX, gridY);

	}


	return this;

}


/**
 * Draw bridge.
 *
 * @param ticks Time
 * @param change Time since last iteration
 */
void JJ1Bridge::draw (unsigned int ticks, int change) {

	unsigned char frame;
	int count;
	fixed bridgeLength, anchorY, leftDipY, rightDipY;


	if (next) next->draw(ticks, change);


	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Check if the event has anything to draw
	if ((animType == E_NOANIM) || ((set->anims[animType] & 0x7F) == 0)) return;


	frame = ticks / (set->animSpeed << 5);

	setAnimFrame(frame + gridX + gridY, true);


	// Draw the bridge

	bridgeLength = set->multiA * set->pieceSize * F4;
	anchorY = getDrawY(change) - F10 - anim->getOffset();

	if (rightDipX >= leftDipX) {

		leftDipY = (leftDipX <= (bridgeLength >> 1)) ? leftDipX >> 3: (bridgeLength - leftDipX) >> 3;
		rightDipY = (rightDipX <= (bridgeLength >> 1)) ? rightDipX >> 3: (bridgeLength - rightDipX) >> 3;

		for (count = 0; count < bridgeLength; count += F4 * set->pieceSize) {

			if (count < leftDipX)
				anim->draw(getDrawX(change) + count, anchorY + (count * leftDipY / leftDipX));
			else if (count < rightDipX)
				anim->draw(getDrawX(change) + count, anchorY + leftDipY + ((count - leftDipX) * (rightDipY - leftDipY) / (rightDipX - leftDipX)));
			else
				anim->draw(getDrawX(change) + count, anchorY + ((bridgeLength - count) * rightDipY / (bridgeLength - rightDipX)));

		}

	} else {

		// No players on the bridge, de-sagging in progress

		// Midpoint
		leftDipY = (leftDipX + rightDipX) >> 1;

		// Dip
		rightDipY = (rightDipX < bridgeLength - leftDipX) ? rightDipX >> 3: (bridgeLength - leftDipX) >> 3;

		for (count = 0; count < bridgeLength; count += F4 * set->pieceSize) {

			if (count < leftDipY)
				anim->draw(getDrawX(change) + count, anchorY + (count * rightDipY / leftDipY));
			else
				anim->draw(getDrawX(change) + count, anchorY + ((bridgeLength - count) * rightDipY / (bridgeLength - leftDipY)));

		}

	}


	return;

}


