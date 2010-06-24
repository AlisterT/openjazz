
/*
 *
 * eventframe.cpp
 *
 * 19th July 2009: Created eventframe.cpp from parts of events.cpp
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
 * Provides the once-per-frame functions of ordinary events.
 *
 */


#include "../bullet.h"
#include "../level.h"
#include "event.h"

#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/levelplayer.h"
#include "util.h"

#include <stdlib.h>


signed char* Event::prepareStep (unsigned int ticks, int msps) {

	signed char* set;

	// Process the next event
	if (next) next = next->step(ticks, msps);


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, destroy it
	if (!set) return NULL;


	// If the event and its origin are off-screen, the event is not in the
	// process of self-destruction, remove it
	if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM) &&
		((x < viewX - F192) || (x > viewX + ITOF(viewW) + F192) ||
		(y < viewY - F160) || (y > viewY + ITOF(viewH) + F160)) &&
		((gridX < FTOT(viewX) - 1) ||
		(gridX > ITOT(FTOI(viewX) + viewW) + 1) ||
		(gridY < FTOT(viewY) - 1) ||
		(gridY > ITOT(FTOI(viewY) + viewH) + 1))) return NULL;


	// Find frame
	if (animType && (set[animType] >= 0)) {

		if ((animType != E_LEFTANIM) && (animType != E_RIGHTANIM))
			frame = (ticks + T_FINISH - level->getEventTime(gridX, gridY)) / 40;
		else if (set[E_ANIMSP])
			frame = ticks / (set[E_ANIMSP] * 40);
		else
			frame = ticks / 20;

	}

	return set;

}


Event* Event::step (unsigned int ticks, int msps) {

	LevelPlayer* levelPlayer;
	fixed width, height;
	signed char* set;
	int count;
	int offset;
	fixed angle;


	set = prepareStep(ticks, msps);

	if (!set) return remove();


	levelPlayer = localPlayer->getLevelPlayer();

	// Find dimensions
	width = getWidth();
	height = getHeight();


	// Handle behaviour

	switch (set[E_BEHAVIOUR]) {

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

			// TODO: Find out what this is

			break;

		case 6:

			// Use the path from the level file
			dx = TTOF(gridX) + F16 + (level->path[set[E_MULTIPURPOSE]].x[level->path[set[E_MULTIPURPOSE]].node] << 9) - x;
			dy = TTOF(gridY) + (level->path[set[E_MULTIPURPOSE]].y[level->path[set[E_MULTIPURPOSE]].node] << 9) - y;
			dx = ((dx << 10) / msps) * set[E_MOVEMENTSP];
			dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

			break;

		case 7:

			// Move back and forth horizontally with tail
			if (animType == E_LEFTANIM) dx = -ES_SLOW;
			else if (animType == E_RIGHTANIM) dx = ES_SLOW;

			break;

		case 8:

			// TODO: Bird-esque following

			break;

		case 9:

			// TODO: Find out what this is

			break;

		case 10:

			// TODO: Find out what this is

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

			// TODO: Move back and forth rapidly

			break;

		case 15:

			// TODO: Rise or lower to meet jazz

			break;

		case 16:

			// Move across level to the left or right
			if (set[E_MAGNITUDE] == 0) dx = -ES_SLOW;
			else dx = set[E_MAGNITUDE] * ES_SLOW;

			break;

		case 17:

			// TODO: Find out what this is

			break;

		case 18:

			// TODO: Find out what this is

			break;

		case 19:

			// TODO: Find out what this is

			break;

		case 20:

			// TODO: Find out what this is

			break;

		case 21:

			// Destructible block
			if (level->getEventHits(gridX, gridY) >= set[E_HITSTOKILL])
				level->setTile(gridX, gridY, set[E_MULTIPURPOSE]);

			break;

		case 22:

			// TODO: Fall down in random spot and repeat

			break;

		case 23:

			// TODO: Find out what this is

			break;

		case 24:

			// TODO: Crawl along ground and go downstairs

			break;

		case 26:

			// TODO: Find out what this is

			break;

		case 27:

			// TODO: Face jazz

			break;

		case 29:

			// Rotate

			offset = set[E_BRIDGELENGTH] * set[E_CHAINLENGTH];
			angle = set[E_MAGNITUDE] * ticks / 13;

			dx = TTOF(gridX) + (fSin(angle) * offset) - x;
			dy = TTOF(gridY) + ((fCos(angle) + F1) * offset) - y;
			dx = ((dx << 10) / msps) * set[E_MOVEMENTSP];
			dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

			break;

		case 30:

			// Swing

			offset = set[E_BRIDGELENGTH] * set[E_CHAINLENGTH];
			angle = (set[E_CHAINANGLE] << 2) + (set[E_MAGNITUDE] * ticks / 13);

			dx = TTOF(gridX) + (fSin(angle) * offset) - x;
			dy = TTOF(gridY) + ((abs(fCos(angle)) + F1) * offset) - y;
			dx = ((dx << 10) / msps) * set[E_MOVEMENTSP];
			dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

			break;

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
					dy = -(F16 + y - (TTOF(gridY) - (set[E_MULTIPURPOSE] * F12))) * 10;
				else
					dy = (F16 + y - (TTOF(gridY) - (set[E_MULTIPURPOSE] * F12))) * 10;

			} else {

				dy = TTOF(gridY) + F16 - y;
				dy = ((dy << 10) / msps) * set[E_MOVEMENTSP];

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

					players[count].getLevelPlayer()->setSpeed(set[E_YAXIS]? set[E_MAGNITUDE] * F4: set[E_MAGNITUDE] * F40,
						set[E_YAXIS]? set[E_MULTIPURPOSE] * -F20: 0);

				}

			}

			break;

		case 39:

			// TODO: Collapsing floor

			break;

		case 40:

			// TODO: Find out what this is

			break;

		case 41:

			// TODO: Switch left & right anim periodically

			break;

		case 42:

			// TODO: Find out what this is

			break;

		case 43:

			// TODO: Find out what this is

			break;

		case 44:

			// TODO: Leap to greet Jazz very quickly

			break;

		case 45:

			// TODO: Find out what this is

			break;

		case 46:

			// TODO: "Final" boss

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

			// TODO: Remaining event behaviours

			break;

	}

	dx /= set[E_MOVEMENTSP];
	dy /= set[E_MOVEMENTSP];
	x += (dx * msps) >> 10;
	y += (dy * msps) >> 10;


	// Choose animation and direction

	if ((animType == E_LEFTANIM) || (animType == E_RIGHTANIM)) {

		switch (set[E_BEHAVIOUR]) {

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

				count = level->path[set[E_MULTIPURPOSE]].node;

				// Check movement direction
				if ((count < 3) ||
					(level->path[set[E_MULTIPURPOSE]].x[count] <= level->path[set[E_MULTIPURPOSE]].x[count - 3]))
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

				if (x < TTOF(gridX) - (set[E_BRIDGELENGTH] << 14))
					animType = E_RIGHTANIM;
				else if (!animType || (x > TTOF(gridX + set[E_BRIDGELENGTH])))
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

					if (y <= F16 + TTOF(gridY) - (set[E_MULTIPURPOSE] * F12))
						animType = E_RIGHTANIM;
					else if (y >= F16 + TTOF(gridY)) {

						animType = E_LEFTANIM;
						level->setEventTime(gridX, gridY, ticks + (set[E_YAXIS] * 50));

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
							(x + width + F4 > viewX + ITOF(viewW)))
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
	if (set[E_HITSTOKILL] &&
		(level->getEventHits(gridX, gridY) >= set[E_HITSTOKILL]) &&
		(animType != E_LFINISHANIM) && (animType != E_RFINISHANIM)) {

		destroy(ticks);

	}


	// Generate bullet
	if (set[E_BULLETSP]) {

		if ((ticks % (set[E_BULLETSP] * 25) >
			(unsigned int)(set[E_BULLETSP] * 25) - T_SHOOT) &&
			((animType == E_LEFTANIM) || (animType == E_RIGHTANIM))) {

			if (animType == E_LEFTANIM) animType = E_LSHOOTANIM;
			else animType = E_RSHOOTANIM;

			level->setEventTime(gridX, gridY, ticks + T_SHOOT);

		}

	}


	// If the reaction time has expired
	if (level->getEventTime(gridX, gridY) &&
		(ticks > level->getEventTime(gridX, gridY))) {

		if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM)) {

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return remove();

		} else if (animType == E_LSHOOTANIM) {

			if ((set[E_BULLET] < 32) &&
				(level->getBullet(set[E_BULLET])[B_SPRITE] != 0))
				level->bullets = new Bullet(this, false, ticks);

			animType = E_LEFTANIM;

		} else if (animType == E_RSHOOTANIM) {

			if ((set[E_BULLET] < 32) &&
				(level->getBullet(set[E_BULLET])[B_SPRITE + 1] != 0))
				level->bullets = new Bullet(this, true, ticks);

			animType = E_RIGHTANIM;

		} else {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	if (level->getStage() == LS_END) return this;

	if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM)) return this;


	// Handle contact with player

	for (count = 0; count < nPlayers; count++) {

		levelPlayer = players[count].getLevelPlayer();

		// Check if the player is touching the event
		if (set[E_MODIFIER] == 6) {

			if (width && height &&
				levelPlayer->overlap(x, y + extraOffset - height, width - F8, F8) &&
				(levelPlayer->getY() <= F8 + ((PYS_FALL * msps) >> 10) + y - height) &&
				!level->checkMaskDown(levelPlayer->getX() + PXO_MID, PYO_TOP + y - height)) {

				// Player is on a platform

				levelPlayer->setEvent(gridX, gridY);
				levelPlayer->setPosition(levelPlayer->getX() + ((dx * msps) >> 10), F4 + y - height);

			} else levelPlayer->clearEvent(gridX, gridY);

		} else {

			// Check if the player is touching the event
			if (width && height &&
				levelPlayer->overlap(x, y + extraOffset - height, width, height)) {

				// If the player picks up the event, destroy it
				if (levelPlayer->touchEvent(gridX, gridY, ticks, msps))
					destroy(ticks);

			}

		}

	}


	return this;

}


void Event::draw (unsigned int ticks, int change) {

	Anim* anim;
	signed char* set;


	if (next) next->draw(ticks, change);


	// Uncomment the following to see the area of the event
	/*drawRect(FTOI(getDrawX(change) - viewX),
		FTOI(getDrawY(change) - (viewY + getHeight())), FTOI(getWidth()),
		FTOI(getHeight()), 88);*/


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


	if (ticks < flashTime) anim->flashPalette(0);

	// Draw the event
	fixed changeX = getDrawX(change);
	fixed changeY = getDrawY(change);

	// Correct the position without altering the animation
	if (noAnimOffset)
		changeY += anim->getOffset();

	if (onlyLAnimOffset && animType == E_RIGHTANIM) {
		changeY += anim->getOffset();
		changeY -= getAnim(E_LEFTANIM)->getOffset();
	}
	if (onlyRAnimOffset && animType == E_LEFTANIM) {
		changeY += anim->getOffset();
		changeY -= getAnim(E_RIGHTANIM)->getOffset();
	}

	anim->draw(changeX, changeY + extraOffset);

	if (ticks < flashTime) anim->restorePalette();


	// If the event has been destroyed, draw an explosion
	if (set[E_HITSTOKILL] &&
		((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))) {

		anim = level->getMiscAnim(2);
		anim->setFrame(frame, false);
		anim->draw(getDrawX(change), getDrawY(change));

	}


	return;

}


void Event::drawEnergy (unsigned int ticks) {

	Anim* anim;
	signed char* set;
	int hits;

	// Get the event properties
	set = level->getEvent(gridX, gridY);

	if (set[E_MODIFIER] != 8) {

		if (next) next->drawEnergy(ticks);

	} else if (set[E_HITSTOKILL]) {

		// Draw boss energy bar

		hits = level->getEventHits(gridX, gridY) * 100 / set[E_HITSTOKILL];


		// Devan head

		anim = level->getMiscAnim(1);
		anim->setFrame(0, true);

		if (ticks < flashTime) anim->flashPalette(0);

		anim->draw(ITOF(viewW - 44), ITOF(hits + 48));

		if (ticks < flashTime) anim->restorePalette();


		// Bar
		drawRect(viewW - 40, hits + 40, 12, 100 - hits, (ticks < flashTime)? 0: 32);

	}

	return;

}


void Event::onlyLeftAnimOffset(bool enable) {

	onlyLAnimOffset = enable;

	return;

}


void Event::onlyRightAnimOffset(bool enable) {

	onlyRAnimOffset = enable;

	return;

}


void Event::noOffset(bool enable) {

	noAnimOffset = enable;

	return;

}


