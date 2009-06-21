
/*
 *
 * events.cpp
 *
 * Created as events.c on the 1st of January 2006 from parts of level.c
 * Renamed events.cpp on the 3rd of February 2009
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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


#include "level.h"
#include "sound.h"
#include <math.h>


Event::Event (unsigned char gX, unsigned char gY, Event *nextEvent) {

	next = nextEvent;
	gridX = gX;
	gridY = gY;
	x = gX << 15;
	y = (gY + 1) << 15;
	flashTime = 0;

	// Choose initial animation and direction

	switch (getProperty(E_BEHAVIOUR)) {

		case 21: // Destructible block

			animType = 0;

			break;

		case 25: // Float up / Belt

			animType = 0;

			break;

		case 26: // Flip animation

			animType = E_RIGHTANIM;

			break;

		case 38: // Sucker tubes

			animType = 0;

			break;

		case 40: // Monochrome

			animType = 0;

			break;

		case 57: // Bubbles

			animType = 0;

			break;

		default:

			animType = E_LEFTANIM;

			break;

	}

	return;

}


Event::~Event () {

	return;

}


Event * Event::getNext () {

	return next;

}


void Event::removeNext () {

	Event *newNext;

	if (next) {

		newNext = next->getNext();
		delete next;
		next = newNext;

	}

	return;

}


bool Event::hit (Player *source, int ticks) {

	int hitsRemaining;

	// Deal with bullet collisions
	if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM) ||
		(ticks < flashTime)) return false;

	hitsRemaining = level->hitEvent(gridX, gridY);

	// If the event cannot be hit, do not register hit
	if (hitsRemaining < 0) return false;

	// Check if the hit has destroyed the event
	if (hitsRemaining == 0) {

		level->setEventTime(gridX, gridY, ticks + T_FINISH);
		animType = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

		level->playSound(getProperty(E_SOUND));

		// Notify the player that shot the bullet
		source->shootEvent(gridX, gridY, ticks);

	}

	// The event has been hit, so it should flash
	flashTime = ticks + T_FLASH;

	// Register hit
	return true;

}


bool Event::isFrom (unsigned char gX, unsigned char gY) {

	return (gX == gridX) && (gY == gridY);

}


fixed Event::getX () {

	return x;

}


fixed Event::getY () {

	return y - getHeight();

}


fixed Event::getWidth () {

	fixed width;

	if (animType && (getProperty(animType) >= 0)) {

		width = level->getAnim(getProperty(animType))->getWidth() << 10;

		// Blank sprites for e.g. invisible springs
		if ((width == F1) && (getHeight() == F1)) return F32;

		return width;

	}

	return F32;

}


fixed Event::getHeight () {

	if (animType && (getProperty(animType) >= 0))
		return level->getAnim(getProperty(animType))->getHeight() << 10;

	return F32;

}


bool Event::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + getWidth() >= left) && (x < left + width) &&
		(y >= top) && (y - getHeight() < top + height);

}


signed char Event::getProperty (unsigned char property) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	if (set) return set[property];

	return 0;

}


bool Event::getFacing () {

	return (animType == E_RIGHTANIM) || (animType == E_RFINISHANIM) ||
		(animType == E_RSHOOTANIM);

}


bool Event::playFrame (int ticks) {

	fixed width, height;
	signed char *set;
	fixed startX;
	int count, offset;
	float angle;

	// Process the next event
	if (next) {

		if (next->playFrame(ticks)) removeNext();

	}


	// Get the event properties
	set = level->getEvent(gridX, gridY);

	// If the event has been removed from the grid, destroy it
	if (!set) return true;


	// If the event and its origin are off-screen, the event is not in the
	// process of self-destruction, remove it
	if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM) &&
		((x < viewX - F160) || (x > viewX + (viewW << 10) + F160) ||
		(y < viewY - F160) || (y > viewY + (viewH << 10) + F160)) &&
		((gridX < (viewX >> 15) - 1) ||
		(gridX > (((viewX >> 10) + viewW) >> 5) + 1) ||
		(gridY < (viewY >> 15) - 1) ||
		(gridY > (((viewY >> 10) + viewH) >> 5) + 1))) return true;


	// Find frame
	if (animType && (set[animType] >= 0)) {

		if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))
			frame = (ticks + T_FINISH - level->getEventTime(gridX, gridY)) / 40;
		else if (set[E_ANIMSP])
			frame = ticks / (set[E_ANIMSP] * 40);
		else frame = ticks / 40;

	}


	// Find dimensions
	width = getWidth();
	height = getHeight();


   	// Pre-movement platform position
	startX = x;


	// Handle behaviour

	switch (set[E_BEHAVIOUR]) {

		case 1: // Sink down

			y += ES_FAST * mspf / set[E_MOVEMENTSP];

			break;

		case 2: // Walk from side to side

			// Walk from side to side
			if (animType == E_LEFTANIM) x -= ES_FAST * mspf / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				x += ES_FAST * mspf / set[E_MOVEMENTSP];

			break;

		case 3: // Seek jazz

			if (localPlayer->getX() + PXO_R < x)
				x -= ES_FAST * mspf / set[E_MOVEMENTSP];
			else if (localPlayer->getX() + PXO_L > x + width)
				x += ES_FAST * mspf / set[E_MOVEMENTSP];

			break;

		case 4: // Walk from side to side and down hills

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				y += ES_FAST * mspf / set[E_MOVEMENTSP];

			} else {

				// Walk from side to side
				if (animType == E_LEFTANIM)
					x -= ES_FAST * mspf / set[E_MOVEMENTSP];
				else if (animType == E_RIGHTANIM)
					x += ES_FAST * mspf / set[E_MOVEMENTSP];

			}

			break;

		case 6: // Use the path from the level file

			x = (gridX << 15) + F16 + (level->pathX[level->pathNode] << 9);
			y = (gridY << 15) + (level->pathY[level->pathNode] << 9);

			break;

		case 7: // Move back and forth horizontally with tail

			if (animType == E_LEFTANIM) x -= ES_SLOW * mspf / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				x += ES_SLOW * mspf / set[E_MOVEMENTSP];

			break;

		case 8: // Bird-esque following

			break;

		case 11: // Sink to ground

			if (!level->checkMaskDown(x + (width >> 1), y))
				y += ES_FAST * mspf / set[E_MOVEMENTSP];

			break;

		case 12: // Move back and forth horizontally

			if (animType == E_LEFTANIM) x -= ES_SLOW * mspf / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				x += ES_SLOW * mspf / set[E_MOVEMENTSP];

			break;

		case 13: // Move up and down

			if (animType == E_LEFTANIM) y -= ES_SLOW * mspf / set[E_MOVEMENTSP];
			else if (animType == E_RIGHTANIM)
				y += ES_SLOW * mspf / set[E_MOVEMENTSP];

			break;

		case 16: // Move across level to the left or right

			x += set[E_MAGNITUDE] * ES_SLOW * mspf / set[E_MOVEMENTSP];

			break;

		case 21: // Destructible block

			if (level->getEventHits(gridX, gridY) >= set[E_HITSTOKILL])
				level->setTile(gridX, gridY, set[E_MULTIPURPOSE]);

			break;

		case 25: // Float up / Belt

			for (count = 0; count < nPlayers; count++) {

				if (players[count].isIn(x, y - height, width, height)) {

					if (set[E_YAXIS]) players[count].floatUp(set);

					players[count].belt(set[E_MAGNITUDE]);

				}

			}

			break;

		case 28: // Bridge

			if (localPlayer->isIn(x - F10,
				y + (set[E_YAXIS] << 10) - F32, set[E_MULTIPURPOSE] * F8,
				F8 + F20)) {

				if (!level->checkMaskDown(localPlayer->getX() + PXO_MID,
					y + (set[E_YAXIS] << 10) - F32 - F20)) {

					// Player is on the bridge

					localPlayer->setEvent(set);
					localPlayer->setPosition(localPlayer->getX(),
						y + (set[E_YAXIS] << 10) - F32);

					offset = (localPlayer->getX() + PXO_MID - x) >> 13;

					if (offset < set[E_MULTIPURPOSE] >> 1) {

						// Player is to the left of the centre of the bridge

						y = (gridY << 15) + F24 + (offset << 10);
						animType = E_LEFTANIM;

					} else {

						// Player is to the right of the centre of the bridge

						y = (gridY << 15) + F24 +
							((set[E_MULTIPURPOSE] - offset) << 10);
						animType = E_RIGHTANIM;

					}

				} else localPlayer->clearEvent(set, E_BEHAVIOUR);

			} else {

				// Player is not on the bridge

				// Gradually stop the bridge sagging
				if (y - F24 > gridY << 15) y -= 16 * mspf;

			}

			break;

		case 29: // Rotate

			offset = (set[E_BRIDGELENGTH] * set[E_CHAINLENGTH]) << 10;
			angle = set[E_MAGNITUDE] * ticks / 2048.0f;

			x = (gridX << 15) + (int)(sin(angle) * offset);
			y = (gridY << 15) + (int)(cos(angle) * offset);

			break;

		case 30: // Swing

			offset = (set[E_BRIDGELENGTH] * set[E_CHAINLENGTH]) << 10;
			angle = (set[E_CHAINANGLE] * 3.141592f / 128.0f) +
				(set[E_MAGNITUDE] * ticks / 2048.0f);

			x = (gridX << 15) + (int)(sin(angle) * offset);
			y = (gridY << 15) + (int)fabs(cos(angle) * offset);

			break;

		case 31: // Move horizontally

			if (animType == E_LEFTANIM) x -= ES_FAST * mspf / set[E_MOVEMENTSP];
			else x += ES_FAST * mspf / set[E_MOVEMENTSP];

			break;

		case 32: // Move horizontally

			if (animType == E_LEFTANIM) x -= ES_FAST * mspf / set[E_MOVEMENTSP];
			else x += ES_FAST * mspf / set[E_MOVEMENTSP];

			break;

		case 33: // Sparks-esque following

			if (localPlayer->getFacing() && (x + width < localPlayer->getX())) {

				x += ES_FAST * mspf / set[E_MOVEMENTSP];

				if (y + height < localPlayer->getY() + PYO_TOP)
					y += ES_SLOW * mspf / set[E_MOVEMENTSP];
				else if (y > localPlayer->getY())
					y -= ES_SLOW * mspf / set[E_MOVEMENTSP];

			} else if (!localPlayer->getFacing() &&
				(x > localPlayer->getX() + F32)) {

				x -= ES_FAST * mspf / set[E_MOVEMENTSP];

				if (y + height < localPlayer->getY() + PYO_TOP)
					y += ES_SLOW * mspf / set[E_MOVEMENTSP];
				else if (y > localPlayer->getY())
					y -= ES_SLOW * mspf / set[E_MOVEMENTSP];

			}

			break;

		case 34: // Launching platform

			if (ticks > level->getEventTime(gridX, gridY)) {

				if (y <= (gridY << 15) + F16 - (set[E_YAXIS] * F2))
					level->setEventTime(gridX, gridY,
						ticks + (set[E_MODIFIER] * 500));
				else
					y -= (y + (set[E_YAXIS] * F2) - (gridY << 15)) * mspf / 320;

			} else {

				if (y < (gridY << 15) + F16)
					y += (y + (set[E_YAXIS] * F2) - (gridY << 15)) * mspf / 320;
				else y = (gridY << 15) + F16;

			}

			break;

		case 36: // Walk from side to side and down hills, staying on-screen

			if (!level->checkMaskDown(x + (width >> 1), y)) {

				// Fall downwards
				y += ES_FAST * mspf / set[E_MOVEMENTSP];

			} else {

				// Walk from side to side, staying on-screen
				if (animType == E_LEFTANIM)
					x -= ES_FAST * mspf / set[E_MOVEMENTSP];
				else if (animType == E_RIGHTANIM)
					x += ES_FAST * mspf / set[E_MOVEMENTSP];

			}

			break;

		case 38: // Sucker tubes

			for (count = 0; count < nPlayers; count++) {

				if (players[count].isIn(x + F8, y - height, width - F16,
					height)) {

					players[count].setSpeed(set[E_MAGNITUDE] * F40,
						set[E_YAXIS]? set[E_MULTIPURPOSE] * -F20: 0);

				}

			}

			break;

		case 53: // Dreempipes turtles

			if (y > level->getWaterLevel(0)) {

				if (animType == E_LEFTANIM)
					x -= ES_SLOW * mspf / set[E_MOVEMENTSP];
				else if (animType == E_RIGHTANIM)
					x += ES_SLOW * mspf / set[E_MOVEMENTSP];

			}

			break;

		/* As yet unhandled event behaviours follow */

		case 14: // Move back and forth rapidly
		case 15: // Rise or lower to meet jazz
		case 22: // Fall down in random spot and repeat
		case 24: // Crawl along ground and go downstairs
		case 27: // Face jazz
		case 39: // Collapsing floor
		case 41: // Switch left & right anim periodically
		case 44: // Leap to greet Jazz very quickly
		case 46: // "Final" boss
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 54:
		case 56:
		case 59:
		case 60:
		case 61:
		case 62:
		case 65:
		default:

			break;

	}


	// Choose animation and direction

	if ((animType == E_LEFTANIM) || (animType == E_RIGHTANIM)) {

		switch (set[E_BEHAVIOUR]) {

			case 2: // Walk from side to side

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

			case 3: // Seek jazz

				if (localPlayer->getX() + PXO_R < x) animType = E_LEFTANIM;
				else if (localPlayer->getX() + PXO_L > x + width)
					animType = E_RIGHTANIM;

				break;

			case 4: // Walk from side to side and down hills

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

			case 6: // Use the path from the level file

				// Check movement direction
				if ((level->pathNode < 3) || (level->pathX[level->pathNode] <=
					level->pathX[level->pathNode - 3])) animType = E_LEFTANIM;
				else animType = E_RIGHTANIM;

				break;

			case 7: // Move back and forth horizontally with tail

				if (animType == E_LEFTANIM) {

					if (x < gridX << 15) animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (x > (gridX << 15) + 100 * F1) animType = E_LEFTANIM;

				}

				break;

			case 12: // Move back and forth horizontally

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x - F4, y - (height >> 1)))
						animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
						animType = E_LEFTANIM;

				}

				break;

			case 13: // Move up and down

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y - height - F4))
						animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + (width >> 1), y + F4))
						animType = E_LEFTANIM;

				}

				break;

			case 26: // Flip animation

				if (localPlayer->isIn(x, y - height, width, height))
					animType = E_LEFTANIM;
				else animType = E_RIGHTANIM;

				break;

			case 31: // Moving platform

				if (animType == E_LEFTANIM) {

					if (level->checkMaskDown(x - F4, y - (height >> 1)))
					    animType = E_RIGHTANIM;

				} else if (animType == E_RIGHTANIM) {

					if (level->checkMaskDown(x + width + F4, y - (height >> 1)))
					    animType = E_LEFTANIM;

				}
 
				break;

			case 32: // Moving platform

				if (x < (gridX << 15) - (set[E_BRIDGELENGTH] << 14))
					animType = E_RIGHTANIM;
				else if (!animType ||
					(x > ((gridX + set[E_BRIDGELENGTH]) << 15)))
					animType = E_LEFTANIM;

				break;

			case 33: // Sparks-esque following

				if (localPlayer->getFacing() &&
					(x + width < localPlayer->getX())) {

					animType = E_RIGHTANIM;

				} else if (!localPlayer->getFacing() &&
					(x > localPlayer->getX() + F32)) {

					animType = E_LEFTANIM;

				}

				break;

			case 36: // Walk from side to side and down hills, staying on-screen

				if (level->checkMaskDown(x + (width >> 1), y)) {

					// Walk from side to side, staying on-screen
					if (animType == E_LEFTANIM) {

						if (level->checkMaskDown(x - F4,
							y - (height >> 1)) || (x - F4 < viewX))
							animType = E_RIGHTANIM;

					} else if (animType == E_RIGHTANIM) {

						if (level->checkMaskDown(x + width + F4,
							y - (height >> 1)) ||
						    (x + width + F4 > viewX + (viewW << 10)))
						    animType = E_LEFTANIM;

					}

				}

				break;

			case 53: // Dreempipes turtles

				if (y > level->getWaterLevel(0)) {

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

		level->setEventTime(gridX, gridY, ticks + T_FINISH);
		animType = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

	}


	// Generate bullet
	if (set[E_BULLETSP]) {

		if ((ticks % (set[E_BULLETSP] * 25) > (set[E_BULLETSP] * 25) - 200) &&
			((animType == E_LEFTANIM) || (animType == E_RIGHTANIM))) {

			if (animType == E_LEFTANIM) animType = E_LSHOOTANIM;
			else animType = E_RSHOOTANIM;

		}

		if ((ticks % (set[E_BULLETSP] * 25) < (set[E_BULLETSP] * 25) - 200) &&
			((animType == E_LSHOOTANIM) || (animType == E_RSHOOTANIM))) {

			level->firstBullet = new Bullet(this, ticks, level->firstBullet);

			if (animType == E_LSHOOTANIM) animType = E_LEFTANIM;
			else animType = E_RIGHTANIM;

		}

	}


	// If the reaction time has expired
	if (level->getEventTime(gridX, gridY) &&
		(ticks > level->getEventTime(gridX, gridY))) {

		if ((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM)) {

			// The event has been destroyed, so remove it
			level->clearEvent(gridX, gridY);

			return true;

		} else {

			// Change the water level
			if (set[E_MODIFIER] == 31) level->setWaterLevel(gridY);

			level->setEventTime(gridX, gridY, 0);

		}

	}


	// Handle contact with player

	for (count = 0; count < nPlayers; count++) {

		if ((animType != E_LFINISHANIM) && (animType != E_RFINISHANIM)) {

			// Check if the player is touching the event

			if (players[count].isIn(x, y - height, width, height)) {

				if (set[E_MODIFIER] == 6) {

					// Platform

					if ((players[count].getY() <=
						y + (PYS_FALL / mspf) - height)
						&& !level->checkMaskDown(players[count].getX() +
						PXO_MID, y - height - F20)) {

						players[count].setEvent(set);
						players[count].setPosition(players[count].getX() +
							x - startX, y - height);

					} else players[count].clearEvent(set, E_MODIFIER);

				}

				// Notify the player
				if (players[count].touchEvent(gridX, gridY, ticks)) {

					// The event has been picked up

					level->setEventTime(gridX, gridY, ticks + T_FINISH);
					animType = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

					level->playSound(set[E_SOUND]);

				}

			}

		}

	}


	return false;

}


void Event::draw (int ticks) {

	Anim *anim;
	signed char *set;
	int count, midpoint;
	fixed bridgex, bridgey, dsty;


	// Uncomment the following to see the area of the event
	/*drawRect((getX() - viewX) >> 10, (getY() - viewY) >> 10, getWidth() >> 10,
		getHeight() >> 10, 88);*/


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

		bridgex = x - F10;
		dsty = y + (set[E_YAXIS] << 10) - F32;

		if (y - F24 > gridY << 15) {

			if (animType == E_LEFTANIM)
				midpoint = (y - F24 - (gridY << 15)) >> 10;
			else midpoint = set[E_MULTIPURPOSE] -
				((y - F24 - (gridY << 15)) >> 10);

		} else midpoint = 0;

		if (midpoint < set[E_MULTIPURPOSE] >> 1) {

			for (count = 0; count < set[E_MULTIPURPOSE]; count++) {

				bridgex += F8;

				if (midpoint == 0) bridgey = dsty;
				else if (count < midpoint)
					bridgey = ((dsty - (midpoint << 10)) *
						(midpoint - count) / midpoint) +
						(dsty * count / midpoint);
				else
					bridgey = ((dsty - (midpoint << 10)) * (count - midpoint) /
						(set[E_MULTIPURPOSE] - midpoint)) +
						(dsty * (set[E_MULTIPURPOSE] - count) /
						(set[E_MULTIPURPOSE] - midpoint));

				anim->draw(bridgex, bridgey);

			}

		} else {

			for (count = 0; count < set[E_MULTIPURPOSE]; count++) {

				bridgex += F8;

				if (midpoint == 0) bridgey = dsty;
				else if (count < midpoint)
					bridgey = ((dsty + ((midpoint - set[E_MULTIPURPOSE]) << 10)) *
						(midpoint - count) / midpoint) +
						(dsty * count / midpoint);
				else
					bridgey = ((dsty + ((midpoint - set[E_MULTIPURPOSE]) << 10)) *
						(count - midpoint) /
						(set[E_MULTIPURPOSE] - midpoint)) +
						(dsty * (set[E_MULTIPURPOSE] - count) /
						(set[E_MULTIPURPOSE] - midpoint));

				anim->draw(bridgex, bridgey);

			}

		}

	} else {

		anim->draw(x, y);

	}

	if (ticks < flashTime) anim->restorePalette();

	// If the event has been destroyed, draw an explosion
	if (set[E_HITSTOKILL] &&
		((animType == E_LFINISHANIM) || (animType == E_RFINISHANIM))) {

		anim = level->getMiscAnim(2);
		anim->setFrame(frame, false);
		anim->draw(x, y);

	}

	if ((set[E_MODIFIER] == 8) && set[E_HITSTOKILL]) {

		// Draw boss energy bar

		count = level->getEventHits(gridX, gridY) * 100 / set[E_HITSTOKILL];


		// Devan head

		anim = level->getMiscAnim(1);
		anim->setFrame(0, true);

		if (ticks < flashTime) anim->flashPalette(0);

		anim->draw(viewX + ((viewW - 44) << 10), viewY + ((count + 48) << 10));

		if (ticks < flashTime) anim->restorePalette();


		// Bar
		drawRect(viewW - 40, count + 40, 12, 100 - count,
			(ticks < flashTime)? 0: 32);

	}


	return;

}


