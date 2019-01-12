
/**
 *
 * @file jj1standardevent.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp
 * - 19th July 2009: Created eventframe.cpp from parts of events.cpp
 * - 2nd March 2010: Created guardians.cpp from parts of event.cpp and eventframe.cpp
 * - 2nd March 2010: Created bridge.cpp from parts of event.cpp and eventframe.cpp
 * - 5th February 2011: Moved parts of eventframe.cpp to event.cpp
 * - 5th February 2011: Renamed eventframe.cpp to standardevent.cpp
 * - 1st August 2012: Renamed standardevent.cpp to jj1standardevent.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Provides the once-per-frame functions of ordinary events.
 *
 */


#include "../jj1bullet.h"
#include "../jj1level.h"
#include "../jj1levelplayer/jj1levelplayer.h"
#include "jj1event.h"

#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "util.h"

#include <stdlib.h>


/**
 * Create standard event.
 *
 * @param event The event type
 * @param gX The grid X-coordinate of the origin of the event
 * @param gY The grid Y-coordinate of the origin of the event
 * @param startX The initial X-coordinate of the event
 * @param startY The initial Y-coordinate of the event
 */
JJ1StandardEvent::JJ1StandardEvent (JJ1EventType* event, unsigned char gX, unsigned char gY, fixed startX, fixed startY) : JJ1Event(gX, gY) {

	set = event;
	x = startX;
	y = startY;

	node = 0;
	onlyLAnimOffset = false;
	onlyRAnimOffset = false;

	switch (set->movement) {

		case 2: // Walk from side to side
		case 4: // Walk from side to side and down hills

			setAnimType(E_LEFTANIM);
			onlyLAnimOffset = true;

			break;

		case 6: // Use the path from the level file
		case 7: // Flying snake behavior

			setAnimType(E_LEFTANIM);
			noAnimOffset = true;

			break;

		case 21: // Destructible block
		case 25: // Float up / Belt
		case 37: // Sucker tubes
		case 38: // Sucker tubes
		case 40: // Monochrome
		case 42: // Reflection
		case 45: // Semitransparency
		case 57: // Bubbles

			animType = E_LEFTANIM;
			setAnimType(E_NOANIM);

			break;

		case 26: // Flip animation

			setAnimType(E_RIGHTANIM);
			onlyRAnimOffset = true;

			break;

		default:

			setAnimType(E_LEFTANIM);

			break;

	}

	return;

}


/**
 * Move standard event.
 *
 * @param ticks Time
 */
void JJ1StandardEvent::move (unsigned int ticks) {

	JJ1LevelPlayer* levelPlayer;
	int length;
	fixed angle;


	if ((animType & ~1) == E_LSHOOTANIM) {

		dx = 0;
		dy = 0;

		return;

	}


	levelPlayer = localPlayer->getJJ1LevelPlayer();


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
		case 7:

			node = (node + FH) % ITOF(level->path[set->multiA].length);

			// Use the path from the level file
			dx = TTOF(gridX) + ITOF(level->path[set->multiA].x[FTOI(node)]) - x;
			dy = TTOF(gridY) + ITOF(level->path[set->multiA].y[FTOI(node)]) - y;

			x += dx;
			y += dy;
			dx = dx << 6;
			dy = dy << 6;

			return;

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
			angle = (set->angle << 2) + (set->magnitude * ticks / 13);

			dx = TTOF(gridX) + (fSin(angle) * length) - x;
			dy = TTOF(gridY) + ((fCos(angle) + F1) * length) - y;

			x += dx;
			y += dy;
			dx = dx << 6;
			dy = dy << 6;

			return;

		case 30:

			// Swing

			length = set->pieceSize * set->pieces;
			angle = (set->angle << 2) + (set->magnitude * ticks / 13);

			dx = TTOF(gridX) + (fSin(angle) * length) - x;
			dy = TTOF(gridY) + ((abs(fCos(angle)) + F1) * length) - y;

			x += dx;
			y += dy;
			dx = dx << 6;
			dy = dy << 6;

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
				dy = dy << 6;

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
			// 37/38: Repel

			/// @todo Remaining event behaviours

			break;

	}


	dx /= set->speed;
	dy /= set->speed;
	x += dx >> 6;
	y += dy >> 6;

	return;

}


/**
 * Event iteration.
 *
 * @param ticks Time
 *
 * @return Remaining event
 */
JJ1Event* JJ1StandardEvent::step (unsigned int ticks) {

	JJ1LevelPlayer* levelPlayer;
	int count;
	unsigned int eventTime;
	int hits;


	set = prepareStep(ticks);
	hits = level->getEventHits(gridX, gridY);

	// If the event is off-screen, remove it (permanently if it's been deflected by a shield)
	if (!set) return remove(hits == 255);


	// If the event has been deflected by a shield, move it
	if (hits == 255) {

		dy = 200 * F1;

		x += dx >> 6;
		y += dy >> 6;

		return this;

	}


	// Get the time of the event's next action
	eventTime = level->getEventTime(gridX, gridY);


	// If the event's finish animation has expired, remove it
	if (eventTime && ((animType & ~1) == E_LFINISHANIM)) {

		if (anim == NULL) return remove(true);

		if (((int)(ticks - eventTime) / (int)(set->animSpeed << 3)) > anim->getLength())
			return remove(true);

	}


	// Get the player
	levelPlayer = localPlayer->getJJ1LevelPlayer();


	// Move
	move(ticks);


	// Choose animation and direction

	if ((animType & ~1) == E_LEFTANIM) {

		switch (set->movement) {

			case 2:

				// Walk from side to side
				if (animType == E_LEFTANIM) {

					if (!level->checkMaskDown(x, y + F4) ||
					    level->checkMaskDown(x - F4, y - (height >> 1)))
						setAnimType(E_RIGHTANIM);

				} else if (animType == E_RIGHTANIM) {

					if (!level->checkMaskDown(x + width, y + F4) ||
					    level->checkMaskDown(x + width + F4, y - (height >> 1)))
						setAnimType(E_LEFTANIM);

				}

				break;

			case 3:

				// Seek jazz
				if (levelPlayer->getX() + PXO_R < x)
					setAnimType(E_LEFTANIM);
				else if (levelPlayer->getX() + PXO_L > x + width)
					setAnimType(E_RIGHTANIM);

				break;

			case 4:

				// Walk from side to side and down hills

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1) - F12))
							setAnimType(E_RIGHTANIM);

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1) - F12))
							setAnimType(E_LEFTANIM);

					}

				}

				break;

			case 6:

				// Use the path from the level file
				// Check movement direction
				if ((FTOI(node) < 3) ||
					(level->path[set->multiA].x[FTOI(node)] <= level->path[set->multiA].x[FTOI(node) - 3]))
					setAnimType(E_LEFTANIM);
				else
					setAnimType(E_RIGHTANIM);

				break;

			case 7:

				// Move back and forth horizontally with tail

				if (animType == E_LEFTANIM) {

					if (x < TTOF(gridX)) setAnimType(E_RIGHTANIM);

				} else if (animType == E_RIGHTANIM) {

					if (x > TTOF(gridX) + F100) setAnimType(E_LEFTANIM);

				}

				break;

			case 12:

				// Move back and forth horizontally

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x - F4, y - (height >> 1)))
						setAnimType(E_RIGHTANIM);

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
						setAnimType(E_LEFTANIM);

				}

				break;

			case 13:

				// Move up and down

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y - height - F4))
						setAnimType(E_RIGHTANIM);

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y + F4))
						setAnimType(E_LEFTANIM);

				}

				break;

			case 26:

				// Flip animation

				if (levelPlayer->overlap(x, y - height, width, height))
					setAnimType(E_LEFTANIM);
				else
					setAnimType(E_RIGHTANIM);

				break;

			case 31:

				// Moving platform

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x, y - (height >> 1)))
						setAnimType(E_RIGHTANIM);

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width, y - (height >> 1)))
						setAnimType(E_LEFTANIM);

				}

				break;

			case 32:

				// Moving platform

				if (x < TTOF(gridX) - (set->pieceSize << 14))
					setAnimType(E_RIGHTANIM);
				else if (x > TTOF(gridX + set->pieceSize))
					setAnimType(E_LEFTANIM);

				break;

			case 33:

				// Sparks-esque following

				if (levelPlayer->getFacing() &&
					(x + width < levelPlayer->getX())) {

					setAnimType(E_RIGHTANIM);

				} else if (!levelPlayer->getFacing() &&
					(x > levelPlayer->getX() + F32)) {

					setAnimType(E_LEFTANIM);

				}

				break;

			case 34:

				// Launching event

				if (ticks > eventTime) {

					if (y <= F16 + TTOF(gridY) - (set->multiA * F12))
						setAnimType(E_RIGHTANIM);
					else if (y >= F16 + TTOF(gridY)) {

						setAnimType(E_LEFTANIM);
						level->setEventTime(gridX, gridY, ticks + (set->multiB * 50));

					}

				} else setAnimType(E_LEFTANIM);

				break;

			case 36:

				// Walk from side to side and down hills, staying on-screen

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side, staying on-screen
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1)) ||
							(x - F4 < viewX))
							setAnimType(E_RIGHTANIM);

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1)) ||
							(x + width + F4 > viewX + ITOF(canvasW)))
							setAnimType(E_LEFTANIM);

					}

				}

				break;

			case 53:

				// Dreempipes turtles

				if (y > level->getWaterLevel()) {

					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4, y - (height >> 1)))
							setAnimType(E_RIGHTANIM);

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
							setAnimType(E_LEFTANIM);

					} else setAnimType(E_LEFTANIM);

				}

				break;

			default:

				if (levelPlayer->getX() + PXO_MID < x + (width >> 1))
					setAnimType(E_LEFTANIM);
				else
					setAnimType(E_RIGHTANIM);

				break;

		}

	}


	// If the event is in its finish stage, nothing else needs to be done
	if ((animType & ~1) == E_LFINISHANIM) return this;


	// If the event has been destroyed, play its finishing animation and set its
	// reaction time
	if (set->strength && (hits >= set->strength)) {

		destroy(ticks);

	}


	if (set->bulletPeriod) {

		count = level->getAnim(set->anims[E_LSHOOTANIM | (animType & 1)])->getLength() * set->animSpeed << 5;

		if ((ticks % (set->bulletPeriod * 32) > (unsigned int)(set->bulletPeriod * 32) - count) &&
			((animType & ~1) == E_LEFTANIM)) {

			// Enter firing mode
			if (animType == E_LEFTANIM) setAnimType(E_LSHOOTANIM);
			else setAnimType(E_RSHOOTANIM);

			level->setEventTime(gridX, gridY, ticks + count);

		}

	}


	// If the reaction time has expired
	if (eventTime && (ticks > eventTime)) {

		if ((animType & ~1) == E_LSHOOTANIM) {

			if (set->bullet < 32)
				level->createBullet(
					NULL,
					gridX,
					gridY,
					drawnX + anim->getShootX(),
					drawnY + anim->getShootY(),
					set->bullet,
					(animType & 1)? true: false,
					ticks);

			setAnimType(E_LEFTANIM | (animType & 1));

		} else {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	if (level->getStage() == LS_END) return this;


	// Handle contact with player

	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getJJ1LevelPlayer();

		// Check if the player is touching the event

		if (set->modifier == 6) {

			if (width && height &&
				levelPlayer->overlap(drawnX, drawnY - F4, width - F8, F8) &&
				(levelPlayer->getY() <= F4 + (PYS_FALL >> 6) + drawnY) &&
				!level->checkMaskDown(levelPlayer->getX() + PXO_MID, PYO_TOP + drawnY)) {

				// Player is on a platform

				levelPlayer->setPlatform(gridX, gridY, (dx >> 6), drawnY);

			} else levelPlayer->clearEvent(gridX, gridY);

		} else {

			// Check if the player is touching the event
			if (width && height &&
				levelPlayer->overlap(drawnX + F2, drawnY + F2, width - F4, height - F4)) {

				// If the player picks up the event, destroy it
				if (levelPlayer->touchEvent(set, gridX, gridY, ticks)) {

					if (level->getEventHits(gridX, gridY) == 255) {

						if (levelPlayer->getX() + PXO_MID > x + (width >> 1))
							dx = -400 * F1;
						else
							dx = 400 * F1;

						dy = 200 * F1;

					} else {

						destroy(ticks);

					}

				}

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
void JJ1StandardEvent::draw (unsigned int ticks, int change) {

	Anim* miscAnim;


	if (next) next->draw(ticks, change);


	// Uncomment the following to see the raw location
	/*drawRect(FTOI(getDrawX(change)),
		FTOI(getDrawY(change) - height), FTOI(width),
		FTOI(height), 88);*/


	// If the event has been removed from the grid, do not show it
	if (!set) return;

	// Check if the event has anything to draw
	if (animType == E_NOANIM) {

		drawnX = x;
		drawnY = y - F32;
		width = F32;
		height = F32;

		return;

	}


	if ((animType & ~1) == E_LFINISHANIM) {

		setAnimFrame((ticks - level->getEventTime(gridX, gridY)) / (set->animSpeed << 3), false);

	} else if ((animType & ~1) == E_LSHOOTANIM) {

		setAnimFrame((ticks + (anim->getLength() * set->animSpeed << 5) - level->getEventTime(gridX, gridY)) / (set->animSpeed << 5), false);

	} else {

		setAnimFrame((ticks / (set->animSpeed << 5)) + gridX + gridY, true);

	}



	// Calculate new positions
	fixed changeX = getDrawX(change);
	fixed changeY = getDrawY(change);


	// Draw the event

	// Check if an explosive effect should be drawn
	if (((animType & ~1) == E_LFINISHANIM) && (set->anims[animType] & 0x80)) {

		// In case of an explosion

		// Determine position in a half circle path
		fixed xOffset = fCos((ticks - level->getEventTime(gridX, gridY)) >> 1) * 48 - ITOF(16);
		fixed yOffset = fSin((ticks - level->getEventTime(gridX, gridY)) >> 1) * 48;

		int val = gridX + gridY;

		// Draw the animation in six different positions
		anim->draw(changeX - yOffset, changeY - xOffset);
		anim->draw(changeX + yOffset, changeY - xOffset);
		anim->draw(changeX + ITOF(val % 32) - yOffset, changeY - ITOF(val % 8) - xOffset);
		anim->draw(changeX - ITOF(val % 16) + yOffset, changeY - ITOF(val % 16) - xOffset);
		anim->draw(changeX + ITOF(val % 24) - yOffset, changeY + ITOF(val % 12) - xOffset);
		anim->draw(changeX - ITOF(val % 48) + yOffset, changeY + ITOF(val % 24) - xOffset);

	} else {

		// In case an event can be drawn normally

		fixed offset;

		if ((ticks < flashTime) && ((ticks >> 4) & 3)) anim->flashPalette(0);

		// Determine the corect vertical offset
		// Most animations need a default offset of 1 tile (32 pixels)

		if ((anim->getWidth() == 1) && (anim->getHeight() == 1)) {

			offset = -F32;

		} else if (noAnimOffset) {

			offset = 0;

		} else {

			if (onlyLAnimOffset && (animType == E_RIGHTANIM)) {

				offset = level->getAnim(set->anims[E_LEFTANIM] & 0x7F)->getOffset();

			} else if (onlyRAnimOffset && (animType == E_LEFTANIM)) {

				offset = level->getAnim(set->anims[E_RIGHTANIM] & 0x7F)->getOffset();

			} else {

				offset = anim->getOffset();

			}

			if (offset == 0) offset = -ITOF(TTOI(1) - 1);

		}

		drawnX = x + anim->getXOffset() + F1;
		drawnY = y + anim->getYOffset() + offset + F1;

		// Uncomment the following line to see the draw area
		//drawRect(FTOI(changeX - x + drawnX), FTOI(changeY - y + drawnY), FTOI(width), FTOI(height), 88);

		anim->draw(changeX + F1, changeY + offset + F1 - anim->getOffset());

		if ((ticks < flashTime) && ((ticks >> 4) & 3)) anim->restorePalette();

	}



	// If the event has been destroyed, draw an explosion
	if (set->strength && ((animType & ~1) == E_LFINISHANIM)) {

		miscAnim = level->getMiscAnim(MA_EXPLOSION1);
		miscAnim->setFrame((ticks - level->getEventTime(gridX, gridY)) >> 3, false);
		miscAnim->draw(changeX, changeY);

	}


	return;

}

