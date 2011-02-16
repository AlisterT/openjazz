
/**
 *
 * @file standardevent.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
 * 2nd March 2010: Created bridge.cpp from parts of event.cpp and eventframe.cpp
 * 5th February 2011: Moved parts of eventframe.cpp to event.cpp
 * 5th February 2011: Renamed eventframe.cpp to standardevent.cpp
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
 * Provides the once-per-frame functions of ordinary events.
 *
 */


#include "../bullet.h"
#include "../level.h"
#include "event.h"

#include "io/gfx/sprite.h"
#include "io/sound.h"
#include "player/levelplayer.h"
#include "util.h"

#include <stdlib.h>


/**
 * Create standard event.
 *
 * @param gX X-coordinate
 * @param gY Y-coordinate
 */
StandardEvent::StandardEvent (unsigned char gX, unsigned char gY) : Event(gX, gY) {

	node = 0;
	onlyLAnimOffset = false;
	onlyRAnimOffset = false;

	switch (set->movement) {

		case 2: // Walk from side to side
		case 4: // Walk from side to side and down hills

			animType = E_LEFTANIM;
			onlyRAnimOffset = true;

			break;

		case 6: // Use the path from the level file
		case 7: // Flying snake behavior

			animType = E_LEFTANIM;
			noAnimOffset = true;

			break;

		case 21: // Destructible block
		case 25: // Float up / Belt
		case 37: // Sucker tubes
		case 38: // Sucker tubes
		case 40: // Monochrome
		case 57: // Bubbles

			animType = E_NOANIM;

			break;

		case 26: // Flip animation

			animType = E_RIGHTANIM;
			onlyLAnimOffset = true;

			break;

		default:

			break;

	}

	return;

}


/**
 * Move standard event.
 *
 * @param ticks Time
 * @param msps Ticks per step
 */
void StandardEvent::move (unsigned int ticks, int msps) {

	LevelPlayer* levelPlayer;
	fixed width, height;
	int count;
	int length;
	fixed angle;


	if ((animType & ~1) == E_LSHOOTANIM) {

		dx = 0;
		dy = 0;

		return;

	}


	levelPlayer = localPlayer->getLevelPlayer();

	// Find dimensions
	width = getWidth();
	height = getHeight();

	// Handle movement

	switch (set->movement) {

		case 1:

			// Sink down
			dy = ES_FAST;

			break;

		case 2:

			// Walk from side to side
			if (animType == E_LEFTANIM) dx = -ES_FAST;
			else if (animType == E_RIGHTANIM) dx = ES_FAST;
			else dx = 0;

			break;

		case 3:

			// Seek jazz
			if (levelPlayer->getX() + PXO_R < x) dx = -ES_FAST;
			else if (levelPlayer->getX() + PXO_L > x + width) dx = ES_FAST;
			else dx = 0;

			break;

		case 4:

			// Walk from side to side and down hills

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				dx = 0;
				dy = ES_FAST;

			} else {

				// Walk from side to side
				if (animType == E_LEFTANIM) dx = -ES_FAST;
				else if (animType == E_RIGHTANIM) dx = ES_FAST;

				dy = 0;

			}

			break;

		case 5:

			/// @todo Find out what behaviour 5 is

			break;

		case 6:

			node = (node + (msps << 5)) % ITOF(level->path[set->multiA].length);

			// Use the path from the level file
			dx = TTOF(gridX) + ITOF(level->path[set->multiA].x[FTOI(node)]) - x;
			dy = TTOF(gridY) + ITOF(level->path[set->multiA].y[FTOI(node)]) - y;

			x += dx;
			y += dy;
			dx = (dx << 10) / msps;
			dy = (dy << 10) / msps;

			return;

		case 7:

			// Move back and forth horizontally with tail
			if (animType == E_LEFTANIM) dx = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dx = ES_SLOW;

			break;

		case 8:

			/// @todo Bird-esque following

			break;

		case 9:

			/// @todo Find out what behaviour 9 is

			break;

		case 10:

			/// @todo Find out what behaviour 10 is

			break;

		case 11:

			// Sink to ground
			if (!level->checkMaskDown(x + (width >> 1), y)) dy = ES_FAST;
			else dy = 0;

			break;

		case 12:

			// Move back and forth horizontally
			if (animType == E_LEFTANIM) dx = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dx = ES_SLOW;
			else dx = 0;

			break;

		case 13:

			// Move up and down
			if (animType == E_LEFTANIM) dy = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dy = ES_SLOW;
			else dy = 0;

			break;

		case 14:

			/// @todo Move back and forth rapidly

			break;

		case 15:

			/// @todo Rise or lower to meet jazz

			break;

		case 16:

			// Move across level to the left or right
			if (set->magnitude == 0) dx = -ES_SLOW;
			else dx = set->magnitude * ES_SLOW;

			break;

		case 17:

			/// @todo Find out what behaviour 17 is

			break;

		case 18:

			/// @todo Find out what behaviour 18 is

			break;

		case 19:

			/// @todo Find out what behaviour 19 is

			break;

		case 20:

			/// @todo Find out what behaviour 20 is

			break;

		case 21:

			// Destructible block
			if (level->getEventHits(gridX, gridY) >= set->strength)
				level->setTile(gridX, gridY, set->multiA);

			break;

		case 22:

			/// @todo Fall down in random spot and repeat

			break;

		case 23:

			/// @todo Find out what behaviour 23 is

			break;

		case 24:

			/// @todo Crawl along ground and go downstairs

			break;

		case 26:

			/// @todo Find out what behaviour 26 is

			break;

		case 27:

			/// @todo Face jazz

			break;

		case 29:

			// Rotate

			length = set->pieceSize * set->pieces;
			angle = set->magnitude * ticks / 13;

			dx = TTOF(gridX) + (fSin(angle) * length) - x;
			dy = TTOF(gridY) + ((fCos(angle) + F1) * length) - y;

			x += dx;
			y += dy;
			dx = (dx << 10) / msps;
			dy = (dy << 10) / msps;

			return;

		case 30:

			// Swing

			length = set->pieceSize * set->pieces;
			angle = (set->angle << 2) + (set->magnitude * ticks / 13);

			dx = TTOF(gridX) + (fSin(angle) * length) - x;
			dy = TTOF(gridY) + ((abs(fCos(angle)) + F1) * length) - y;

			x += dx;
			y += dy;
			dx = (dx << 10) / msps;
			dy = (dy << 10) / msps;

			return;

		case 31:

			// Move horizontally
			if (animType == E_LEFTANIM) dx = -ES_FAST;
			else dx = ES_FAST;

			break;

		case 32:

			// Move horizontally
			if (animType == E_LEFTANIM) dx = -ES_FAST;
			else dx = ES_FAST;

			break;

		case 33:

			// Sparks-esque following

			if (levelPlayer->getFacing() && (x + width < levelPlayer->getX())) {

				dx = ES_FAST;

				if (y + height < levelPlayer->getY() + PYO_TOP) dy = ES_SLOW;
				else if (y > levelPlayer->getY()) dy = -ES_SLOW;
				else dy = 0;

			} else if (!levelPlayer->getFacing() && (x > levelPlayer->getX() + F32)) {

				dx = -ES_FAST;

				if (y + height < levelPlayer->getY() + PYO_TOP) dy = ES_SLOW;
				else if (y > levelPlayer->getY()) dy = -ES_SLOW;
				else dy = 0;

			} else {

				dx = 0;
				dy = 0;

			}

			break;

		case 34:

			// Launching event

			if (ticks > level->getEventTime(gridX, gridY)) {

				if (animType == E_LEFTANIM)
					dy = -(F16 + y - (TTOF(gridY) - (set->multiA * F12))) * 10;
				else
					dy = (F16 + y - (TTOF(gridY) - (set->multiA * F12))) * 10;

			} else {

				dy = TTOF(gridY) + F16 - y;

				y += dy;
				dy = ((dy << 10) / msps);

				return;

			}

			break;

		case 35:

			// Non-floating Sparks-esque following

			if (levelPlayer->getFacing() && (x + width < levelPlayer->getX() + PXO_L - F4)) {

				if (level->checkMaskDown(x + width, y + F4) &&
					!level->checkMaskDown(x + width + F4, y - (height >> 1)))
					dx = ES_FAST;
				else
					dx = 0;

			} else if (!levelPlayer->getFacing() && (x > levelPlayer->getX() + PXO_R + F4)) {

				if (level->checkMaskDown(x, y + F4) &&
				    !level->checkMaskDown(x - F4, y - (height >> 1)))
			    	dx = -ES_FAST;
			    else
			    	dx = 0;

			} else dx = 0;

			break;

		case 36:

			// Walk from side to side and down hills, staying on-screen

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				dx = 0;
				dy = ES_FAST;

			} else {

				// Walk from side to side, staying on-screen
				if (animType == E_LEFTANIM) dx = -ES_FAST;
				else if (animType == E_RIGHTANIM) dx = ES_FAST;
				else dx = 0;

				dy = 0;

			}

			break;

		case 37:
		case 38:

			// Sucker tubes

			for (count = 0; count < nPlayers; count++) {

				if (players[count].getLevelPlayer()->overlap(x + F8, y + F4 - height, width - F16,
					height - F8)) {

					players[count].getLevelPlayer()->setSpeed(
						set->multiB? set->magnitude * F4: set->magnitude * F40,
						set->multiB? set->multiA * -F20: 0);

				}

			}

			break;

		case 39:

			/// @todo Collapsing floor

			break;

		case 40:

			/// @todo Find out what behaviour 40 is

			break;

		case 41:

			/// @todo Switch left & right anim periodically

			break;

		case 42:

			/// @todo Find out what behaviour 42 is

			break;

		case 43:

			/// @todo Find out what behaviour 43 is

			break;

		case 44:

			/// @todo Leap to greet Jazz very quickly

			break;

		case 45:

			/// @todo Find out what behaviour 45 is

			break;

		case 46:

			/// @todo "Final" boss

			break;

		case 53:

			// Dreempipes turtles

			if (y > level->getWaterLevel()) {

				if (animType == E_LEFTANIM) dx = -ES_SLOW;
				else if (animType == E_RIGHTANIM) dx = ES_SLOW;
				else dx = 0;

			} else dx = 0;

			break;

		default:

			// Do nothing for the following:
			// 0: Static
			// 25: Float up / Belt

			/// @todo Remaining event behaviours

			break;

	}


	dx /= set->speed;
	dy /= set->speed;
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;

	return;

}


/**
 * Event iteration.
 *
 * @param ticks Time
 * @param msps Ticks per step
 *
 * @return Remaining event
 */
Event* StandardEvent::step (unsigned int ticks, int msps) {

	LevelPlayer* levelPlayer;
	fixed width, height;
	int count;


	set = prepareStep(ticks, msps);

	if (!set) return remove();


	levelPlayer = localPlayer->getLevelPlayer();

	// Find dimensions
	width = getWidth();
	height = getHeight();

	// Move
	move(ticks, msps);

	// Choose animation and direction

	if ((animType & ~1) == E_LEFTANIM) {

		switch (set->movement) {

			case 2:

				// Walk from side to side
				if (animType == E_LEFTANIM) {

					if (!level->checkMaskDown(x, y + F4) ||
					    level->checkMaskDown(x - F4, y - (height >> 1)))
					    animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (!level->checkMaskDown(x + width, y + F4) ||
					    level->checkMaskDown(x + width + F4, y - (height >> 1)))
					    animType = E_LEFTANIM;

				}

				break;

			case 3:

				// Seek jazz
				if (levelPlayer->getX() + PXO_R < x)
					animType = E_LEFTANIM;
				else if (levelPlayer->getX() + PXO_L > x + width)
					animType = E_RIGHTANIM;

				break;

			case 4:

				// Walk from side to side and down hills

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1) - F12))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1) - F12))
							animType = E_LEFTANIM;

					}

				}

				break;

			case 6:

				// Use the path from the level file
				// Check movement direction
				if ((FTOI(node) < 3) ||
					(level->path[set->multiA].x[FTOI(node)] <= level->path[set->multiA].x[FTOI(node) - 3]))
					animType = E_LEFTANIM;
				else
					animType = E_RIGHTANIM;

				break;

			case 7:

				// Move back and forth horizontally with tail

				if (animType == E_LEFTANIM) {

					if (x < TTOF(gridX)) animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (x > TTOF(gridX) + F100) animType = E_LEFTANIM;

				}

				break;

			case 12:

				// Move back and forth horizontally

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x - F4, y - (height >> 1)))
						animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
						animType = E_LEFTANIM;

				}

				break;

			case 13:

				// Move up and down

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y - height - F4))
						animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y + F4))
						animType = E_LEFTANIM;

				}

				break;

			case 26:

				// Flip animation

				if (levelPlayer->overlap(x, y - height, width, height))
					animType = E_LEFTANIM;
				else
					animType = E_RIGHTANIM;

				break;

			case 31:

				// Moving platform

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x, y - (height >> 1)))
					    animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width, y - (height >> 1)))
					    animType = E_LEFTANIM;

				}

				break;

			case 32:

				// Moving platform

				if (x < TTOF(gridX) - (set->pieceSize << 14))
					animType = E_RIGHTANIM;
				else if (x > TTOF(gridX + set->pieceSize))
					animType = E_LEFTANIM;

				break;

			case 33:

				// Sparks-esque following

				if (levelPlayer->getFacing() &&
					(x + width < levelPlayer->getX())) {

					animType = E_RIGHTANIM;

				} else if (!levelPlayer->getFacing() &&
					(x > levelPlayer->getX() + F32)) {

					animType = E_LEFTANIM;

				}

				break;

			case 34:

				// Launching event

				if (ticks > level->getEventTime(gridX, gridY)) {

					if (y <= F16 + TTOF(gridY) - (set->multiA * F12))
						animType = E_RIGHTANIM;
					else if (y >= F16 + TTOF(gridY)) {

						animType = E_LEFTANIM;
						level->setEventTime(gridX, gridY, ticks + (set->multiB * 50));

					}

				} else animType = E_LEFTANIM;

				break;

			case 36:

				// Walk from side to side and down hills, staying on-screen

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side, staying on-screen
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1)) ||
							(x - F4 < viewX))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1)) ||
							(x + width + F4 > viewX + ITOF(320)))
							animType = E_LEFTANIM;

					}

				}

				break;

			case 53:

				// Dreempipes turtles

				if (y > level->getWaterLevel()) {

					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1)))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
							animType = E_LEFTANIM;

					} else animType = E_LEFTANIM;

				}

				break;

			default:

				if (levelPlayer->getX() + PXO_MID < x + (width >> 1))
					animType = E_LEFTANIM;
				else
					animType = E_RIGHTANIM;

				break;

		}

	}

	// If the event has been destroyed, play its finishing animation and set its
	// reaction time
	if (set->strength &&
		(level->getEventHits(gridX, gridY) >= set->strength) &&
		((animType & ~1) != E_LFINISHANIM)) {

		destroy(ticks);

	}


	if (set->bulletPeriod) {

		count = level->getAnim(set->anims[E_LSHOOTANIM | (animType & 1)])->getLength() * set->animSpeed << 5;

		if ((ticks % (set->bulletPeriod * 32) > (unsigned int)(set->bulletPeriod * 32) - count) &&
			((animType & ~1) == E_LEFTANIM)) {

			// Enter firing mode
			if (animType == E_LEFTANIM) animType = E_LSHOOTANIM;
			else animType = E_RSHOOTANIM;

			level->setEventTime(gridX, gridY, ticks + count);

		}

	}


	// If the reaction time has expired
	if (level->getEventTime(gridX, gridY) &&
		(ticks > level->getEventTime(gridX, gridY))) {

		if ((animType & ~1) == E_LFINISHANIM) {

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return remove();

		} else if ((animType & ~1) == E_LSHOOTANIM) {

			if ((set->bullet < 32) &&
				(level->getBullet(set->bullet)[B_SPRITE | (animType & 1)] != 0))
				level->bullets = new Bullet(
					x + getAnim()->getShootX(),
					y + getAnim()->getShootY() - F4,
					set->bullet, (animType & 1)? true: false, ticks);

			animType = E_LEFTANIM | (animType & 1);

		} else {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	if (level->getStage() == LS_END) return this;

	if ((animType & ~1) == E_LFINISHANIM) return this;


	// Handle contact with player

	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getLevelPlayer();

		// Check if the player is touching the event

		fixed offset = 0;

		if ((animType != E_NOANIM) && getAnim() && noAnimOffset)
			offset = getAnim()->getOffset();

		if (set->modifier == 6) {

			if (width && height &&
				levelPlayer->overlap(x, y + offset - height, width - F8, F8) &&
				(levelPlayer->getY() <= F8 + ((PYS_FALL * msps) >> 10) + y - height) &&
				!level->checkMaskDown(levelPlayer->getX() + PXO_MID, PYO_TOP + y - height)) {

				// Player is on a platform

				levelPlayer->setEvent(gridX, gridY);
				levelPlayer->setPosition(levelPlayer->getX() + ((dx * msps) >> 10), F4 + y - height);

			} else levelPlayer->clearEvent(gridX, gridY);

		} else {

			// Check if the player is touching the event
			if (width && height &&
				levelPlayer->overlap(x, y + offset - height, width, height)) {

				// If the player picks up the event, destroy it
				if (levelPlayer->touchEvent(gridX, gridY, ticks, msps))
					destroy(ticks);

			}

		}

	}


	return this;

}


/**
 * Draw standard event.
 *
 * @param ticks Time
 * @param change Time since last iteration
 */
void StandardEvent::draw (unsigned int ticks, int change) {

	Anim* anim;
	unsigned char frame;


	if (next) next->draw(ticks, change);


	// Uncomment the following to see the area of the event
	/*drawRect(FTOI(getDrawX(change)),
		FTOI(getDrawY(change) - getHeight()), FTOI(getWidth()),
		FTOI(getHeight()), 88);*/


	// If the event has been removed from the grid, do not show it
	if (!set) return;

	// Check if the event has anything to draw
	if (animType == E_NOANIM) return;


	// Decide on the frame to draw
	anim = getAnim();

	if ((animType & ~1) == E_LFINISHANIM) {

		frame = (ticks + (anim->getLength() * set->animSpeed << 3) - level->getEventTime(gridX, gridY)) / (set->animSpeed << 3);

	} else if ((animType & ~1) == E_LSHOOTANIM) {

		frame = (ticks + (anim->getLength() * set->animSpeed << 5) - level->getEventTime(gridX, gridY)) / (set->animSpeed << 5);

	} else {

		frame = (ticks / (set->animSpeed << 5)) + gridX + gridY;

	}

	anim->setFrame(frame, true);


	// Calculate new positions
	fixed changeX = getDrawX(change);
	fixed changeY = getDrawY(change);


	// Correct the position without altering the animation
	if (noAnimOffset) {

		changeY += anim->getOffset();

	}
	else if (onlyLAnimOffset && animType == E_RIGHTANIM) {

		changeY += anim->getOffset();
		changeY -= level->getAnim(set->anims[E_LEFTANIM] & 0x7F)->getOffset();

	}
	else if (onlyRAnimOffset && animType == E_LEFTANIM) {

		changeY += anim->getOffset();
		changeY -= level->getAnim(set->anims[E_RIGHTANIM] & 0x7F)->getOffset();

	}


	// Draw the event

	// Check if an explosive effect should be drawn
	if (((animType & ~1) == E_LFINISHANIM) && (set->anims[animType] & 0x80)) {

		// In case of an explosion

		// Determine position in a half circle path
		fixed xOffset = fSin(level->getEventTime(gridX, gridY) - ticks) * 48 - ITOF(16);
		fixed yOffset = fCos(level->getEventTime(gridX, gridY) - ticks) * 48;

		int val = gridX + gridY;

		// Draw the animation in six different positions
		anim->draw(changeX - yOffset, changeY - xOffset);
		anim->draw(changeX + yOffset, changeY - xOffset);
		anim->draw(changeX + ITOF(val % 32) - yOffset, changeY - ITOF(val % 8) - xOffset);
		anim->draw(changeX - ITOF(val % 16) + yOffset, changeY - ITOF(val % 16) - xOffset);
		anim->draw(changeX + ITOF(val % 24) - yOffset, changeY + ITOF(val % 12) - xOffset);
		anim->draw(changeX - ITOF(val % 48) + yOffset, changeY + ITOF(val % 24) - xOffset);

	}
	else {

		// In case an event can be drawn normally

		if (ticks < flashTime) anim->flashPalette(0);

		anim->draw(changeX, changeY);

	}


	if (ticks < flashTime) anim->restorePalette();


	// If the event has been destroyed, draw an explosion
	if (set->strength && ((animType & ~1) == E_LFINISHANIM)) {

		anim = level->getMiscAnim(2);
		anim->setFrame(frame, false);
		anim->draw(changeX, changeY);

	}


	return;

}

