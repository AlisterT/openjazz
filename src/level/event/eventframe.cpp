
/*
 *
 * eventframe.cpp
 *
 * 19th July 2009: Created eventframe.cpp from parts of events.cpp
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
 * Provides the once-per-frame functions of events.
 *
 */


#include "../bullet.h"
#include "../level.h"
#include "event.h"

#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/player.h"

#include <math.h>


bool Event::step (unsigned int ticks, int msps) {

	fixed width, height;
	signed char *set;
	int count;
	fixed offset;
	float angle;

	// Process the next event
	if (next) {

		if (next->step(ticks, msps)) removeNext();

	}


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, destroy it
	if (!set) return true;


	// If the event and its origin are off-screen, the event is not in the
	// process of self-destruction, remove it
	if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM) &&
		((x < viewX - F160) || (x > viewX + ITOF(viewW) + F160) ||
		(y < viewY - F160) || (y > viewY + ITOF(viewH) + F160)) &&
		((gridX < FTOT(viewX) - 1) ||
		(gridX > ITOT(FTOI(viewX) + viewW) + 1) ||
		(gridY < FTOT(viewY) - 1) ||
		(gridY > ITOT(FTOI(viewY) + viewH) + 1))) return true;


	// Find frame
	if (animType && (set[animType] >= 0)) {

		if ((animType != E_LEFTANIM) && (animType != E_RIGHTANIM))
			frame = (ticks + T_FINISH - level->getEventTime(gridX, gridY)) / 40;
		else if (set[E_ANIMSP])
			frame = ticks / (set[E_ANIMSP] * 40);
		else frame = ticks / 40;

	}


	// Find dimensions
	width = getWidth();
	height = getHeight();


	// Handle behaviour

	switch (set[E_BEHAVIOUR]) {

		case 1:

			// Sink down
			dy = ES_FAST * msps / set[E_MOVEMENTSP];

			break;

		case 2:

			// Walk from side to side
			if (animType == E_LEFTANIM)
				dx = -ES_FAST * msps / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				dx = ES_FAST * msps / set[E_MOVEMENTSP];
			else dx = 0;

			break;

		case 3:

			// Seek jazz
			if (localPlayer->getX() + PXO_R < x)
				dx = -ES_FAST * msps / set[E_MOVEMENTSP];
			else if (localPlayer->getX() + PXO_L > x + width)
				dx = ES_FAST * msps / set[E_MOVEMENTSP];
			else dx = 0;

			break;

		case 4:

			// Walk from side to side and down hills

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				dx = 0;
				dy = ES_FAST * msps / set[E_MOVEMENTSP];

			} else {

				// Walk from side to side
				if (animType == E_LEFTANIM)
					dx = -ES_FAST * msps / set[E_MOVEMENTSP];
				else if (animType == E_RIGHTANIM)
					dx = ES_FAST * msps / set[E_MOVEMENTSP];

				dy = 0;

			}

			break;

		case 5:

			// TODO: Find out what this is

			break;

		case 6:

			// Use the path from the level file
			dx = TTOF(gridX) + F16 + (level->pathX[level->pathNode] << 9) - x;
			dy = TTOF(gridY) + (level->pathY[level->pathNode] << 9) - y;

			break;

		case 7:

			// Move back and forth horizontally with tail
			if (animType == E_LEFTANIM)
				dx = -ES_SLOW * msps / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				dx = ES_SLOW * msps / set[E_MOVEMENTSP];

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
			if (!level->checkMaskDown(x + (width >> 1), y))
				dy = ES_FAST * msps / set[E_MOVEMENTSP];
			else dy = 0;

			break;

		case 12:

			// Move back and forth horizontally
			if (animType == E_LEFTANIM)
				dx = -ES_SLOW * msps / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				dx = ES_SLOW * msps / set[E_MOVEMENTSP];
			else dx = 0;

			break;

		case 13:

			// Move up and down
			if (animType == E_LEFTANIM)
				dy = -ES_SLOW * msps / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				dy = ES_SLOW * msps / set[E_MOVEMENTSP];
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
			if (set[E_MAGNITUDE] == 0) dx = -ES_SLOW * msps / set[E_MOVEMENTSP];
			else dx = set[E_MAGNITUDE] * ES_SLOW * msps / set[E_MOVEMENTSP];

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

		case 25:

			// Float up / Belt

			for (count = 0; count < nPlayers; count++) {

				if (players[count].overlap(x, y - height, width, height)) {

					if (set[E_YAXIS]) players[count].floatUp(set, msps);

					players[count].belt(set[E_MAGNITUDE] * msps);

				}

			}

			break;

		case 26:

			// TODO: Find out what this is

			break;

		case 27:

			// TODO: Face jazz

			break;

		case 28:

			// Bridge

			// Gradually stop the bridge sagging
			if (dx < set[E_MULTIPURPOSE] * F8) dx += 320 * msps;
			if (dx > set[E_MULTIPURPOSE] * F8) dx = set[E_MULTIPURPOSE] * F8;
			if (dy > 0) dy -= 320 * msps;
			if (dy < 0) dy = 0;

			for (count = 0; count < nPlayers; count++) {

				if (players[count].overlap(x, y, set[E_MULTIPURPOSE] * F8,
					F8)) {

					offset = players[count].getX() + PXO_MID;

					if (!level->checkMaskDown(offset, y - F32)) {

						// Player is on the bridge

						players[count].setEvent(set);

						offset -= x;

						if (offset < dx) dx = offset;

						if ((offset > dy) &&
							(offset < set[E_MULTIPURPOSE] * F8)) dy = offset;

						if (offset < set[E_MULTIPURPOSE] * F4)
							players[count].setPosition(players[count].getX(),
								y + (offset >> 3) - F8);
						else
							players[count].setPosition(players[count].getX(),
								y + (set[E_MULTIPURPOSE] * F1) - (offset >> 3) - F8);

					} else players[count].clearEvent(set, E_BEHAVIOUR);

				}

			}

			break;

		case 29:

			// Rotate

			offset = ITOF(set[E_BRIDGELENGTH] * set[E_CHAINLENGTH]);
			angle = set[E_MAGNITUDE] * ticks / 2048.0f;

			dx = TTOF(gridX) + (int)(sin(angle) * offset) - x;
			dy = TTOF(gridY) + (int)((cos(angle) + 1.0f) * offset) - y;

			break;

		case 30:

			// Swing

			offset = ITOF(set[E_BRIDGELENGTH] * set[E_CHAINLENGTH]);
			angle = (set[E_CHAINANGLE] * 3.141592f / 128.0f) +
				(set[E_MAGNITUDE] * ticks / 2048.0f);

			dx = TTOF(gridX) + (int)(sin(angle) * offset) - x;
			dy = TTOF(gridY) + (int)fabs((cos(angle) + 1.0f) * offset) - y;

			break;

		case 31:

			// Move horizontally
			if (animType == E_LEFTANIM) dx = -ES_FAST * msps / set[E_MOVEMENTSP];
			else dx = ES_FAST * msps / set[E_MOVEMENTSP];

			break;

		case 32:

			// Move horizontally
			if (animType == E_LEFTANIM) dx = -ES_FAST * msps / set[E_MOVEMENTSP];
			else dx = ES_FAST * msps / set[E_MOVEMENTSP];

			break;

		case 33:

			// Sparks-esque following

			if (localPlayer->getFacing() && (x + width < localPlayer->getX())) {

				dx = ES_FAST * msps / set[E_MOVEMENTSP];

				if (y + height < localPlayer->getY() + PYO_TOP)
					dy = ES_SLOW * msps / set[E_MOVEMENTSP];
				else if (y > localPlayer->getY())
					dy = -ES_SLOW * msps / set[E_MOVEMENTSP];
				else dy = 0;

			} else if (!localPlayer->getFacing() &&
				(x > localPlayer->getX() + F32)) {

				dx = -ES_FAST * msps / set[E_MOVEMENTSP];

				if (y + height < localPlayer->getY() + PYO_TOP)
					dy = ES_SLOW * msps / set[E_MOVEMENTSP];
				else if (y > localPlayer->getY())
					dy = -ES_SLOW * msps / set[E_MOVEMENTSP];
				else dy = 0;

			} else {

				dx = 0;
				dy = 0;

			}

			break;

		case 34:

			// Launching platform

			if (ticks > level->getEventTime(gridX, gridY)) {

				if (y <= TTOF(gridY) + F16 - (set[E_YAXIS] * F2)) {

					level->setEventTime(gridX, gridY, ticks + (set[E_MOVEMENTSP] * 1000));
					dy = 0;

				} else dy = -(y + (set[E_YAXIS] * F2) - TTOF(gridY)) * msps / 320;

			} else {

				if (y < TTOF(gridY) + F16) dy = (y + (set[E_YAXIS] * F2) - TTOF(gridY)) * msps / 320;
				else dy = (TTOF(gridY) + F16) - y;

			}

			break;

		case 35:

			// Non-floating Sparks-esque following

			if (localPlayer->getFacing() && (x + width < localPlayer->getX() + PXO_L - F4)) {

				if (level->checkMaskDown(x + width, y + F4) &&
					!level->checkMaskDown(x + width + F4, y - (height >> 1)))
					dx = ES_FAST * msps / set[E_MOVEMENTSP];
				else dx = 0;

			} else if (!localPlayer->getFacing() && (x > localPlayer->getX() + PXO_R + F4)) {

				if (level->checkMaskDown(x, y + F4) &&
				    !level->checkMaskDown(x - F4, y - (height >> 1)))
				    dx = -ES_FAST * msps / set[E_MOVEMENTSP];
			    else dx = 0;

			} else dx = 0;

			break;

		case 36:

			// Walk from side to side and down hills, staying on-screen

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				dx = 0;
				dy = ES_FAST * msps / set[E_MOVEMENTSP];

			} else {

				// Walk from side to side, staying on-screen
				if (animType == E_LEFTANIM)
					dx = -ES_FAST * msps / set[E_MOVEMENTSP];
				else if (animType == E_RIGHTANIM)
					dx = ES_FAST * msps / set[E_MOVEMENTSP];
				else dx = 0;

				dy = 0;

			}

			break;

		case 37:
		case 38:

			// Sucker tubes

			for (count = 0; count < nPlayers; count++) {

				if (players[count].overlap(x + F8, y + F4 - height, width - F16,
					height - F8)) {

					players[count].setSpeed(set[E_YAXIS]? set[E_MAGNITUDE] * F4: set[E_MAGNITUDE] * F40,
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

				if (animType == E_LEFTANIM)
					dx = -ES_SLOW * msps / set[E_MOVEMENTSP];
				else if (animType == E_RIGHTANIM)
					dx = ES_SLOW * msps / set[E_MOVEMENTSP];
				else dx = 0;

			} else dx = 0;

			break;

		default:

			// TODO: Remaining event behaviours

			break;

	}


	// Apply movement
	if (set[E_BEHAVIOUR] != 28) {

		x += dx;
		y += dy;

	}


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
				if (localPlayer->getX() + PXO_R < x) animType = E_LEFTANIM;
				else if (localPlayer->getX() + PXO_L > x + width)
					animType = E_RIGHTANIM;

				break;

			case 4:

				// Walk from side to side and down hills

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4,
							y - (height >> 1) - F12)) animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4,
							y - (height >> 1) - F12)) animType = E_LEFTANIM;

					}

				}

				break;

			case 6:

				// Use the path from the level file

				// Check movement direction
				if ((level->pathNode < 3) || (level->pathX[level->pathNode] <=
					level->pathX[level->pathNode - 3])) animType = E_LEFTANIM;
				else animType = E_RIGHTANIM;

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

				if (localPlayer->overlap(x, y - height, width, height))
					animType = E_LEFTANIM;
				else animType = E_RIGHTANIM;

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
				else if (!animType ||
					(x > TTOF(gridX + set[E_BRIDGELENGTH])))
					animType = E_LEFTANIM;

				break;

			case 33:

				// Sparks-esque following

				if (localPlayer->getFacing() &&
					(x + width < localPlayer->getX())) {

					animType = E_RIGHTANIM;

				} else if (!localPlayer->getFacing() &&
					(x > localPlayer->getX() + F32)) {

					animType = E_LEFTANIM;

				}

				break;

			case 36:

				// Walk from side to side and down hills, staying on-screen

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side, staying on-screen
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4,
							y - (height >> 1)) || (x - F4 < viewX))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4,
							y - (height >> 1)) ||
						    (x + width + F4 > viewX + ITOF(viewW)))
						    animType = E_LEFTANIM;

					}

				}

				break;

			case 53:

				// Dreempipes turtles

				if (y > level->getWaterLevel()) {

					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4,
							y - (height >> 1))) animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4,
							y - (height >> 1))) animType = E_LEFTANIM;

					} else animType = E_LEFTANIM;

				}

				break;

			default:

				if (localPlayer->getX() + PXO_MID < x + (width >> 1))
					animType = E_LEFTANIM;
				else animType = E_RIGHTANIM;

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

			// Change the water level
			if (set[E_MODIFIER] == 31) level->setWaterLevel(gridY);

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return true;

		} else if (animType == E_LSHOOTANIM) {

			if ((set[E_BULLET] < 32) &&
				(level->getBullet(set[E_BULLET])[B_SPRITE] != 0))
				level->firstBullet = new Bullet(this, false, ticks);

			animType = E_LEFTANIM;

		} else if (animType == E_RSHOOTANIM) {

			if ((set[E_BULLET] < 32) &&
				(level->getBullet(set[E_BULLET])[B_SPRITE + 1] != 0))
				level->firstBullet = new Bullet(this, true, ticks);

			animType = E_RIGHTANIM;

		} else {

			level->setEventTime(gridX, gridY, 0);

		}

	}


	if (level->getStage() == LS_END) return false;


	// Handle contact with player

	if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM)) {

		for (count = 0; count < nPlayers; count++) {

			// Check if the player is touching the event

			if (width && height &&
				players[count].overlap(x, y - height, width, height)) {

				if (set[E_MODIFIER] == 6) {

					// Platform

					if ((players[count].getY() <=
						y + (PYS_FALL / msps) - height)
						&& !level->checkMaskDown(players[count].getX() +
						PXO_MID, y - height - F20)) {

						players[count].setEvent(set);
						players[count].setPosition(players[count].getX() +
							dx, y - height);

					} else players[count].clearEvent(set, E_MODIFIER);

				}

				// If the player picks up the event, destroy it
				if (players[count].touchEvent(gridX, gridY, ticks))
					destroy(ticks);

			}

		}

	}


	return false;

}


void Event::draw (unsigned int ticks, int change) {

	Anim *anim;
	signed char *set;
	int count;
	fixed bridgeLength, dipA, dipB;


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
	else frame = ticks / 20;

	anim = level->getAnim(set[animType]);
	anim->setFrame(frame + gridX + gridY, true);

	if (ticks < flashTime) anim->flashPalette(0);

	// Draw the event

	if (set[E_BEHAVIOUR] == 28) {

		bridgeLength = set[E_MULTIPURPOSE] * F8;

		if (dy >= dx) {

			dipA = (dx <= (bridgeLength >> 1)) ? dx >> 3:
				(bridgeLength - dx) >> 3;
			dipB = (dy <= (bridgeLength >> 1)) ? dy >> 3:
				(bridgeLength - dy) >> 3;

			for (count = 0; count < bridgeLength; count += F8) {

				if (count < dx)
					anim->draw(x + count, y + (count * dipA / dx));
				else if (count < dy)
					anim->draw(x + count,
						y + dipA +
							((count - dx) * (dipB - dipA) / (dy - dx)));
				else
					anim->draw(x + count,
						y + ((bridgeLength - count) * dipB /
							(bridgeLength - dy)));

			}

		} else {

			// No players on the bridge, de-sagging in progress

			dipA = (dx + dy) >> 1;
			dipB = (dy < bridgeLength - dx) ? dy >> 3: (bridgeLength - dx) >> 3;

			for (count = 0; count < bridgeLength; count += F8) {

				if (count < dipA)
					anim->draw(x + count, y + (count * dipB / dipA));
				else
					anim->draw(x + count,
						y + ((bridgeLength - count) * dipB /
							(bridgeLength - dipA)));

			}

		}

	} else {

		anim->draw(getDrawX(change), getDrawY(change));

	}

	if (ticks < flashTime) anim->restorePalette();

	// If the event has been destroyed, draw an explosion
	if (set[E_HITSTOKILL] &&
		((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))) {

		anim = level->getMiscAnim(2);
		anim->setFrame(frame, false);
		anim->draw(getDrawX(change), getDrawY(change));

	}

	if ((set[E_MODIFIER] == 8) && set[E_HITSTOKILL]) {

		// Draw boss energy bar

		count = level->getEventHits(gridX, gridY) * 100 / set[E_HITSTOKILL];


		// Devan head

		anim = level->getMiscAnim(1);
		anim->setFrame(0, true);

		if (ticks < flashTime) anim->flashPalette(0);

		anim->draw(viewX + ITOF(viewW - 44), viewY + ITOF(count + 48));

		if (ticks < flashTime) anim->restorePalette();


		// Bar
		drawRect(viewW - 40, count + 40, 12, 100 - count,
			(ticks < flashTime)? 0: 32);

	}


	return;

}


