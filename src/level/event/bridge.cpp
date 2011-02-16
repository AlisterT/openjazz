
/**
 *
 * @file bridge.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 2nd March 2010: Created bridge.cpp from parts of event.cpp and eventframe.cpp
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
 * Provides the functions of bridge events.
 *
 */


#include "../level.h"
#include "event.h"

#include "player/levelplayer.h"


/**
 * Create bridge.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
Bridge::Bridge (unsigned char gX, unsigned char gY) : Event(gX, gY) {

	y = TTOF(gY) + ITOF(set->multiB);

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
 * @param msps Ticks per step
 *
 * @return Remaining event
 */
Event* Bridge::step (unsigned int ticks, int msps) {

	LevelPlayer* levelPlayer;
	int count;
	fixed bridgeLength, playerDipX, playerDipY;


	set = prepareStep(ticks, msps);

	if (!set) return remove();


	bridgeLength = set->multiA * set->pieceSize * F4;


	// Gradually stop the bridge sagging
	if (leftDipX < bridgeLength) leftDipX += 320 * msps;
	if (leftDipX > bridgeLength) leftDipX = bridgeLength;
	if (rightDipX > 0) rightDipX -= 320 * msps;
	if (rightDipX < 0) rightDipX = 0;


	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getLevelPlayer();

		playerDipX = levelPlayer->getX() + PXO_MID - x;

		if (playerDipX < bridgeLength >> 1) playerDipY = playerDipX >> 3;
		else playerDipY = (bridgeLength - playerDipX) >> 3;

		if (levelPlayer->overlap(x, y - F8 + playerDipY - F4, bridgeLength, F8) &&
			!level->checkMaskDown(x + playerDipX, y - F8 + playerDipY - F32)) {

			// Player is on the bridge

			levelPlayer->setEvent(gridX, gridY);

			if (playerDipX < leftDipX) leftDipX = playerDipX;

			if (playerDipX > rightDipX) rightDipX = playerDipX;

			levelPlayer->setPosition(levelPlayer->getX(), y - F8 - F1 + playerDipY);

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
void Bridge::draw (unsigned int ticks, int change) {

	Anim* anim;
	unsigned char frame;
	int count;
	fixed bridgeLength, anchorY, leftDipY, rightDipY;


	if (next) next->draw(ticks, change);


	// If the event has been removed from the grid, do not show it
	if (!set) return;


	// Check if the event has anything to draw
	if ((animType == E_NOANIM) || ((set->anims[animType] & 0x7F) == 0)) return;


	frame = ticks / (set->animSpeed << 5);

	anim = getAnim();
	anim->setFrame(frame + gridX + gridY, true);


	// Draw the bridge

	bridgeLength = set->multiA * set->pieceSize * F4;
	anchorY = getDrawY(change) - F10 + anim->getOffset();

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


