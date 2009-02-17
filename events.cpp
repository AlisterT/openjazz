
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


#include "OpenJazz.h"


Event::Event (unsigned char gX, unsigned char gY, Event *nextEvent) {

	next = nextEvent;
	gridX = gX;
	gridY = gY;
	x = gX << 15;
	y = (gY + 1) << 15;
	anim = 0;
	flashTime = 0;

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

bool Event::isFrom (unsigned char gX, unsigned char gY) {

	return (gX == gridX) && (gY == gridY);

}


fixed Event::getX () {

	if (anim && (getProperty(anim) >= 0))
		return x +
			(levelInst->getAnim(getProperty(anim))->sprites[frame].x << 10);

	return x;

}


fixed Event::getY () {

	if (anim && (getProperty(anim) >= 0))
		return y +
			((levelInst->getAnim(getProperty(anim))->sprites[frame].y +
			levelInst->getAnim(getProperty(anim))->y[frame] -
			levelInst->getAnim(getProperty(anim))->sprites[0].pixels->h) << 10);

	return y - F32;

}


fixed Event::getWidth () {

	fixed width;

	if (anim && (getProperty(anim) >= 0)) {

		width =
			levelInst->getAnim(getProperty(anim))->sprites[0].pixels->w << 10;

		// Blank sprites for e.g. invisible springs
		if ((width == F1) && (getHeight() == F1)) return F32;

		return width;

	}

	return F32;

}


fixed Event::getHeight () {

	if (anim && (getProperty(anim) >= 0))
		return (levelInst->getAnim(getProperty(anim))->sprites[0].pixels->h -
			(levelInst->getAnim(getProperty(anim))->sprites[frame].y +
			levelInst->getAnim(getProperty(anim))->y[frame])) << 10;

	return F32;

}


signed char Event::getProperty (unsigned char property) {

	return levelInst->getEvent(gridX, gridY)[property];

}


bool Event::getFacing () {

	return (anim == E_RIGHTANIM) || (anim == E_RFINISHANIM) ||
		(anim == E_RSHOOTANIM);

}


bool Event::playFrame (int ticks) {

	struct {

		fixed x, y, w, h;

	} pos;
	Bullet *bul, *prevBul;
	Player *localPlayer;
	GridElement *ge;
	signed char *set;
	fixed startX, targetH;
	int midpoint;

	// Process the next event
	if (next) {

		if (next->playFrame(ticks)) removeNext();

	}


	// Get the player
	localPlayer = players;

	// Get the grid element at the given coordinates
	ge = levelInst->getGrid(gridX, gridY);

	// Get the event properties
	set = levelInst->getEvent(gridX, gridY);


	// If the event and its origin are off-screen, the event is not in the
	// process of self-destruction, and it is not a bird, remove it
	if ((anim != E_LFINISHANIM) && (anim != E_RFINISHANIM) &&
		(ge->event != 121) &&
	    ((x < localPlayer->viewX - F160) ||
	     (x > localPlayer->viewX +
	               (localPlayer->viewW << 10) + F160) ||
	     (y < localPlayer->viewY - F160) ||
	     (y > localPlayer->viewY +
	               (localPlayer->viewH << 10) + F160)   ) &&
	    ((gridX < (localPlayer->viewX >> 15) - 1) ||
	     (gridX >
	      (((localPlayer->viewX >> 10) + localPlayer->viewW) >> 5) + 1) ||
	     (gridY < (localPlayer->viewY >> 15) - 1) ||
	     (gridY >
	      (((localPlayer->viewY >> 10) + localPlayer->viewH) >> 5) + 1)   ))
		return true;


	// Find frame
	if (anim && (set[anim] >= 0)) {

		if ((anim == E_LFINISHANIM) || (anim == E_RFINISHANIM))
			frame = ((ticks + 200 - ge->time) / 40) %
				levelInst->getAnim(set[anim])->frames;
		else if (set[E_ANIMSP])
			frame = (ticks / (set[E_ANIMSP] * 40)) %
				levelInst->getAnim(set[anim])->frames;
		else frame = (ticks / 40) % levelInst->getAnim(set[anim])->frames;

	}


	// Find dimensions
	pos.x = getX();
	pos.y = getY();
	pos.w = getWidth();
	pos.h = getHeight();

	// Height as a target
    if (anim && (set[anim] >= 0))
    	targetH = levelInst->getFrame(set[anim], 0)->pixels->h << 10;
   	else targetH = F32;


   	// Pre-movement platform position
	startX = x;


	// Handle behaviour
	switch (set[E_BEHAVIOUR]) {

		case 1: // Sink down

			y += 320 * mspf / set[E_MOVEMENTSP];

			break;

		case 2: // Walk from side to side

			// Walk from side to side
			if (anim == E_LEFTANIM) x -= 320 * mspf / set[E_MOVEMENTSP];
			else if (anim == E_RIGHTANIM) x += 320 * mspf / set[E_MOVEMENTSP];

			break;

		case 3: // Seek jazz

			if (localPlayer->getX() + PXO_R < pos.x)
				x -= 320 * mspf / set[E_MOVEMENTSP];
			else if (localPlayer->getX() + PXO_L > pos.x + pos.w)
				x += 320 * mspf / set[E_MOVEMENTSP];

			break;

		case 4: // Walk from side to side and down hills

			if (!levelInst->checkMaskDown(pos.x + (pos.w >> 1),
				pos.y + pos.h)) {

				// Fall downwards
				y += 320 * mspf / set[E_MOVEMENTSP];

			} else {

				// Walk from side to side
				if (anim == E_LEFTANIM) x -= 320 * mspf / set[E_MOVEMENTSP];
				else if (anim == E_RIGHTANIM)
					x += 320 * mspf / set[E_MOVEMENTSP];

			}

			break;

		case 6: // Use the path from the level file

			x = (gridX << 15) + F16 +
				(levelInst->pathX[levelInst->pathNode] << 11);
			y = (gridY << 15) + (levelInst->pathY[levelInst->pathNode] << 11);

			break;

		case 7: // Move back and forth horizontally with tail

			if (anim == E_LEFTANIM) x -= 80 * mspf / set[E_MOVEMENTSP];
			else if (anim == E_RIGHTANIM) x += 80 * mspf / set[E_MOVEMENTSP];

			break;

		case 8: // Bird-esque following

			if ((ge->event == 121) && (ge->hits == 255)) {

				// Fly away

				x += 80 * mspf / set[E_MOVEMENTSP];
				y -= 80 * mspf / set[E_MOVEMENTSP];

			} else {

				// Fly towards the player

				if (x < localPlayer->getX())
					x += 160 * mspf / set[E_MOVEMENTSP];
				else if (x > localPlayer->getX())
					x -= 160 * mspf / set[E_MOVEMENTSP];

				if (y > localPlayer->getY() - F64)
					y -= 160 * mspf / set[E_MOVEMENTSP];
				else if (y < localPlayer->getY() - F64)
					y += 160 * mspf / set[E_MOVEMENTSP];

				if (y + pos.h > levelInst->getWaterLevel(ticks))
					y = levelInst->getWaterLevel(ticks) - pos.h;

			}

			break;

		case 11: // Sink to ground

			if (!levelInst->checkMaskDown(pos.x + (pos.w >> 1), pos.y + pos.h))
				y += 320 * mspf / set[E_MOVEMENTSP];

			break;

		case 12: // Move back and forth horizontally

			if (anim == E_LEFTANIM) x -= 80 * mspf / set[E_MOVEMENTSP];
			else if (anim == E_RIGHTANIM) x += 80 * mspf / set[E_MOVEMENTSP];

			break;

		case 13: // Move up and down

			if (anim == E_LEFTANIM) y -= 80 * mspf / set[E_MOVEMENTSP];
			else if (anim == E_RIGHTANIM) y += 80 * mspf / set[E_MOVEMENTSP];

			break;

		case 16: // Move across level to the left or right

			x += set[E_MAGNITUDE] * 80 * mspf / set[E_MOVEMENTSP];

			break;

		case 21: // Destructible block

			if (ge->hits >= set[E_HITSTOKILL]) ge->tile = set[E_MULTIPURPOSE];

			break;

		case 28: // Bridge

			if (localPlayer->isIn(pos.x - F10,
				pos.y + (set[E_YAXIS] << 10) - F32, set[E_MULTIPURPOSE] * F8,
				F8 + F20)) {

				if (!levelInst->checkMaskDown(localPlayer->getX() + PXO_MID,
					pos.y + (set[E_YAXIS] << 10) - (F32 + F20))) {

					// Player is on the bridge

					localPlayer->setEvent(set);
					localPlayer->setPosition(localPlayer->getX(),
						pos.y + (set[E_YAXIS] << 10) - F32);

					midpoint = (localPlayer->getX() + PXO_MID - pos.x) >> 13;

					if (midpoint < set[E_MULTIPURPOSE] / 2) {

						// Player is to the left of the centre of the bridge

						y = ((gridY + 1) << 15) + (midpoint << 10);
						anim = E_LEFTANIM;

					} else {

						// Player is to the right of the centre of the bridge

						y = ((gridY + 1) << 15) +
							((set[E_MULTIPURPOSE] - midpoint) << 10);
						anim = E_RIGHTANIM;

					}

				} else localPlayer->clearEvent(set, E_BEHAVIOUR);

			} else {

				// Player is not on the bridge

				// Gradually stop the bridge sagging
				midpoint = 0;
				if (y > (gridY + 1) << 15) y -= 16 * mspf;

			}

			break;

		case 31: // Moving platform

			if (anim == E_LEFTANIM) x -= 320 * mspf / set[E_MOVEMENTSP];
			else x += 320 * mspf / set[E_MOVEMENTSP];

			break;

		case 33: // Sparks-esque following

			if (localPlayer->getFacing() &&
				(pos.x + pos.w < localPlayer->getX())) {

				x += 320 * mspf / set[E_MOVEMENTSP];

				if (pos.y + pos.h < localPlayer->getY() + PYO_TOP)
					y += 80 * mspf / set[E_MOVEMENTSP];
				else if (pos.y > localPlayer->getY())
					y -= 80 * mspf / set[E_MOVEMENTSP];

			} else if (!localPlayer->getFacing() &&
				(pos.x > localPlayer->getX() + F32)) {

				x -= 320 * mspf / set[E_MOVEMENTSP];

				if (pos.y + pos.h < localPlayer->getY() + PYO_TOP)
					y += 80 * mspf / set[E_MOVEMENTSP];
				else if (pos.y > localPlayer->getY())
					y -= 80 * mspf / set[E_MOVEMENTSP];

			}

			break;

		case 34: // Launching platform

			if (ticks > ge->time) {

				if (y <= (gridY << 15) + F16 - (set[E_YAXIS] * F2))
					ge->time = ticks + (set[E_MODIFIER] * 500);
				else
					y -= (y + (set[E_YAXIS] * F2) - (gridY << 15)) * mspf / 320;

			} else {

				if (y < (gridY << 15) + F16)
					y += (y + (set[E_YAXIS] * F2) - (gridY << 15)) * mspf / 320;
				else y = (gridY << 15) + F16;

			}

			break;

		case 36: // Walk from side to side and down hills, staying on-screen

			if (!levelInst->checkMaskDown(pos.x + (pos.w >> 1),
				pos.y + pos.h)) {

				// Fall downwards
				y += 320 * mspf / set[E_MOVEMENTSP];

			} else {

				// Walk from side to side, staying on-screen
				if (anim == E_LEFTANIM) x -= 320 * mspf / set[E_MOVEMENTSP];
				else if (anim == E_RIGHTANIM)
					x += 320 * mspf / set[E_MOVEMENTSP];

			}

			break;

		/* As yet unhandled event behaviours follow */

		case 14: // Move back and forth rapidly
		case 15: // Rise or lower to meet jazz
		case 22: // Fall down in random spot and repeat
		case 24: // Crawl along ground and go downstairs
		case 27: // Face jazz
		case 29: // Nonmoving object with jazz
		case 30: // Nonmoving object with jazz
		case 32: // Nonmoving object
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
		case 53:
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
	if (!anim || (anim == E_LEFTANIM) || (anim == E_RIGHTANIM)) {

		switch (set[E_BEHAVIOUR]) {

			case 2: // Walk from side to side

				// Walk from side to side
				if (anim == E_LEFTANIM) {

					if (!levelInst->checkMaskDown(pos.x, pos.y + pos.h + F4) ||
					    levelInst->checkMaskDown(pos.x - F4,
					    pos.y + (pos.h >> 1))) anim = E_RIGHTANIM;

				} else if (anim == E_RIGHTANIM) {

					if (!levelInst->checkMaskDown(pos.x + pos.w,
						pos.y + pos.h + F4) ||
					    levelInst->checkMaskDown(pos.x + pos.w + F4,
					    pos.y + (pos.h >> 1))) anim = E_LEFTANIM;

				} else anim = E_LEFTANIM;

				break;

			case 3: // Seek jazz

				if (localPlayer->getX() + PXO_R < pos.x) anim = E_LEFTANIM;
				else if (localPlayer->getX() + PXO_L > pos.x + pos.w)
					anim = E_RIGHTANIM;
				else if (!anim) anim = E_RIGHTANIM;

				break;

			case 4: // Walk from side to side and down hills

				if (levelInst->checkMaskDown(pos.x + (pos.w >> 1), pos.y + pos.h)) {

					// Walk from side to side
					if (anim == E_LEFTANIM) {

						if (levelInst->checkMaskDown(pos.x - F4, pos.y))
							anim = E_RIGHTANIM;

					} else if (anim == E_RIGHTANIM) {

						if (levelInst->checkMaskDown(pos.x + pos.w + F4, pos.y))
							anim = E_LEFTANIM;

					} else anim = E_LEFTANIM;

				}

				break;

			case 6: // Use the path from the level file

				// Check movement direction
				if ((levelInst->pathNode < 3) ||
				    (levelInst->pathX[levelInst->pathNode] <=
				    levelInst->pathX[levelInst->pathNode - 3]))
					anim = E_LEFTANIM;
				else anim = E_RIGHTANIM;

				break;

			case 7: // Move back and forth horizontally with tail

				if (anim == E_LEFTANIM) {

					if (x < gridX << 15) anim = E_RIGHTANIM;

				} else if (anim == E_RIGHTANIM) {

					if (x > (gridX << 15) + 100 * F1) anim = E_LEFTANIM;

				} else anim = E_LEFTANIM;

				break;

			case 8: // Bird-esque following

				if ((ge->event != 121) || (ge->hits != 255)) {

					anim = localPlayer->getFacing()? E_RIGHTANIM: E_LEFTANIM;

				}

				break;

			case 12: // Move back and forth horizontally

				if (anim == E_LEFTANIM) {

					if (levelInst->checkMaskDown(pos.x - F4,
						pos.y + (pos.h >> 1))) anim = E_RIGHTANIM;

				} else if (anim == E_RIGHTANIM) {

					if (levelInst->checkMaskDown(pos.x + pos.w + F4,
						pos.y + (pos.h >> 1))) anim = E_LEFTANIM;

				} else anim = E_LEFTANIM;

				break;

			case 13: // Move up and down

				if (anim == E_LEFTANIM) {

					if (levelInst->checkMaskDown(pos.x + (pos.w >> 1),
						pos.y - F4)) anim = E_RIGHTANIM;

				} else if (anim == E_RIGHTANIM) {

					if (levelInst->checkMaskDown(pos.x + (pos.w >> 1),
						pos.y + pos.h + F4)) anim = E_LEFTANIM;

				} else anim = E_LEFTANIM;

				break;

			case 16: // Move across level to the left or right

				anim = E_LEFTANIM;

				break;

			case 21: // Destructible block

				break;

			case 25: // Float up / Belt

				if (localPlayer->isIn(pos.x, pos.y, pos.w, pos.h)) {

					if (set[E_YAXIS])
						localPlayer->floatUp(set);

					localPlayer->belt(set[E_MAGNITUDE]);

				}

				break;

			case 26: // Flip animation

				if (localPlayer->isIn(pos.x, pos.y, pos.w, pos.h))
					anim = E_LEFTANIM;
				else anim = E_RIGHTANIM;

				break;

			case 28: // Bridge

				if (!anim) anim = E_LEFTANIM;

				break;

			case 31: // Moving platform

				if (x < (gridX << 15) - (set[E_BRIDGELENGTH] << 14))
					anim = E_RIGHTANIM;
				else if (!anim || (x > ((gridX + set[E_BRIDGELENGTH]) << 15)))
					anim = E_LEFTANIM;

				break;

			case 33: // Sparks-esque following

				if (localPlayer->getFacing() &&
					(pos.x + pos.w < localPlayer->getX())) {

					anim = E_RIGHTANIM;

				} else if (!localPlayer->getFacing() &&
					(pos.x > localPlayer->getX() + F32)) {

					anim = E_LEFTANIM;

				} else if (!anim) anim = E_LEFTANIM;

				break;

			case 36: // Walk from side to side and down hills, staying on-screen

				if (levelInst->checkMaskDown(pos.x + (pos.w >> 1),
					pos.y + pos.h)) {

					// Walk from side to side, staying on-screen
					if (anim == E_LEFTANIM) {

						if (levelInst->checkMaskDown(pos.x - F4,
							pos.y + (pos.h >> 1)) ||
							(pos.x - F4 < localPlayer->viewX))
							anim = E_RIGHTANIM;

					} else if (anim == E_RIGHTANIM) {

						if (levelInst->checkMaskDown(pos.x + pos.w + F4,
							pos.y + (pos.h >> 1)) ||
						    (pos.x + pos.w + F4 > localPlayer->viewX +
						    (localPlayer->viewW << 10))) anim = E_LEFTANIM;

					} else anim = E_LEFTANIM;

				}

				break;

			case 38: // Sucker tubes

				if (localPlayer->isIn(pos.x + F4, pos.y + F4, pos.w - F16,
					pos.h - F16)) {

					localPlayer->setSpeed(set[E_MAGNITUDE] * F40,
						set[E_YAXIS]? set[E_MULTIPURPOSE] * -F40: 0);

				}

				break;

			default:

				if (localPlayer->getX() + PXO_MID < pos.x + (pos.w >> 1))
					anim = E_LEFTANIM;
				else anim = E_RIGHTANIM;

				break;

		}

	}


	// Deal with bullet collisions
	if ((anim != E_LFINISHANIM) && (anim != E_RFINISHANIM) &&
		(ge->hits < set[E_HITSTOKILL])) {

		bul = levelInst->firstBullet;
		prevBul = NULL;

		while (bul) {

			if (bul->getSource() && bul->isIn(pos.x, pos.y, pos.w, targetH)) {

				ge->hits++;
				flashTime = ticks + FLASHTIME;

				if (!prevBul) {

					bul = bul->getNext();
					delete levelInst->firstBullet;
					levelInst->firstBullet = bul;

				} else {

					bul = bul->getNext();
					prevBul->removeNext();

				}

			} else {

				prevBul = bul;
				bul = bul->getNext();

			}

		}

	}


	// Generate bullet
	if (set[E_BULLETSP]) {

		if ((ticks % (set[E_BULLETSP] * 25) > (set[E_BULLETSP] * 25) - 200) &&
			((anim == E_LEFTANIM) || (anim == E_RIGHTANIM))) {

			if (anim == E_LEFTANIM) anim = E_LSHOOTANIM;
			else anim = E_RSHOOTANIM;

		}

		if ((ticks % (set[E_BULLETSP] * 25) < (set[E_BULLETSP] * 25) - 200) &&
			((anim == E_LSHOOTANIM) || (anim == E_RSHOOTANIM))) {

			levelInst->firstBullet = new Bullet(this, ticks,
				levelInst->firstBullet);

			if (anim == E_LSHOOTANIM) anim = E_LEFTANIM;
			else anim = E_RIGHTANIM;

		}

	}


	// If the event has been destroyed, play its finishing animation and set its
	// reaction time
	if (set[E_HITSTOKILL] && (ge->hits >= set[E_HITSTOKILL]) &&
		(anim != E_LFINISHANIM) && (anim != E_RFINISHANIM)) {

		localPlayer->addScore(set[E_ADDEDSCORE]);
		ge->time = ticks + 200;
		anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

	}


	// If the reaction time has expired
	if (ge->time && (ticks > ge->time)) {

		// Handle modifiers which take effect after reaction time
		switch (set[E_MODIFIER]) {

			case 41: // Bonus level

				if (localPlayer->getEnergy())
					levelInst->setNext(set[E_MULTIPURPOSE], set[E_YAXIS]);

				// The lack of a break statement is intentional

			case 8: // Boss
			case 27: // End of level

				if (localPlayer->getEnergy()) {

					localPlayer->setCheckpoint(gridX, gridY);
					localPlayer->win(ticks);

				}

				break;

			case 10: // Checkpoint

				localPlayer->setCheckpoint(gridX, gridY);

				break;

			case 13: // Warp

				localPlayer->setPosition(set[E_MULTIPURPOSE] << 15,
					(set[E_YAXIS] + 1) << 15);
				ge->time = 0;

				break;

			case 15:

				localPlayer->addAmmo(0, 15);

				break;

			case 16:

				localPlayer->addAmmo(1, 15);

				break;

			case 17:

				localPlayer->addAmmo(2, 15);

				break;

			case 31: // Water level

				levelInst->setWaterLevel(gridY);

				break;

			case 33:

				localPlayer->addShield(false);

				break;

			case 34: // Bird

				// Turn the event into a bird
				ge->event = 121;
				ge->hits = localPlayer->getEnergy();

				return true;

			case 36:

				localPlayer->addShield(true);

				break;

		}

		if ((anim == E_LFINISHANIM) || (anim == E_RFINISHANIM)) {

			// If the event hurts and can be killed, it is an enemy
			// Anything else that scores is an item
			if ((set[E_MODIFIER] == 0) && set[E_HITSTOKILL])
				localPlayer->enemies++;
			else if (set[E_ADDEDSCORE]) localPlayer->items++;

			ge->event = 0;

			return true;

		} else {

			ge->time = 0;

		}

	}


	// Handle contact with player
	if ((anim != E_LFINISHANIM) && (anim != E_RFINISHANIM) &&
		localPlayer->isIn(pos.x, pos.y, pos.w, pos.h)) {

		// Handle modifiers which take effect on contact
		switch (set[E_MODIFIER]) {

			case 0: // Hurt
			case 8: // Boss

				if ((set[E_BEHAVIOUR] < 37) || (set[E_BEHAVIOUR] > 44))
					localPlayer->hit(ticks);

				break;

			case 1: // Invincibility

				if (localPlayer->getEnergy()) {

                    localPlayer->addStar(ticks);
					localPlayer->addScore(set[E_ADDEDSCORE]);

					ge->time = ticks + 200;
					anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				}

				break;

			case 2:
			case 3: // Health

				localPlayer->addCarrot();
				localPlayer->addScore(set[E_ADDEDSCORE]);

				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 4: // Extra life

				localPlayer->addLife();
				localPlayer->addScore(set[E_ADDEDSCORE]);

				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 5: // High-jump feet

				localPlayer->addFeet();

				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 6: // Platform

				if ((localPlayer->getY() <= pos.y + (PYS_FALL / mspf)) &&
				    !levelInst->checkMaskDown(localPlayer->getX() + PXO_MID,
				    pos.y - F20)) {

					localPlayer->setEvent(set);
					localPlayer->setPosition(localPlayer->getX() + x - startX,
						pos.y);

				} else localPlayer->clearEvent(set, E_MODIFIER);

				break;

			case 9: // Sand timer

				levelInst->addTimer();
				localPlayer->addScore(set[E_ADDEDSCORE]);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 11: // Item

				localPlayer->addScore(set[E_ADDEDSCORE]);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 12: // Rapid fire

				localPlayer->addRapidFire();
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 13: // Warp

				ge->time = ticks + 1000;

				break;

			case 18:

				localPlayer->addAmmo(0, 2);
				localPlayer->addScore(set[E_ADDEDSCORE]);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 19:

				localPlayer->addAmmo(1, 2);
				localPlayer->addScore(set[E_ADDEDSCORE]);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 20:

				localPlayer->addAmmo(2, 2);
				localPlayer->addScore(set[E_ADDEDSCORE]);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 29: // Upwards spring

				localPlayer->setEvent(set);

				break;

			case 30:

				localPlayer->addAmmo(3, 1);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 31: // Water level

				if (!set[E_HITSTOKILL]) levelInst->setWaterLevel(gridY);

				break;

			case 35: // Airboard, etc.

				localPlayer->setFloating(true);

				localPlayer->addScore(set[E_ADDEDSCORE]);
				ge->time = ticks + 200;
				anim = getFacing()? E_RFINISHANIM: E_LFINISHANIM;

				break;

			case 38: // Airboard, etc. off

				localPlayer->setFloating(false);

				break;

		}

	}


	// Handle birds
	if (ge->event == 121) {

		// Set to fly away if the player has been hit
		if (localPlayer->getEnergy() < ge->hits) ge->hits = 255;
		else ge->hits = localPlayer->getEnergy();

	}

	return false;

}


void Event::draw (int ticks) {

	SDL_Rect dst;
	SDL_Surface *sprite;
	Player *localPlayer;
	GridElement *ge;
	signed char *set;
	int count, dstx, dsty, midpoint;


	// Get the player
	localPlayer = players;

	// Get the grid element at the given coordinates
	ge = levelInst->getGrid(gridX, gridY);

	// Check whether or not the event should be processed
	if (!ge->event || (ge->event >= 122)) return;

	set = levelInst->getEvent(gridX, gridY);


	// Check if the event has anything to draw
	if (!anim || (set[anim] < 0)) return;


	if ((anim == E_LFINISHANIM) || (anim == E_RFINISHANIM))
		frame = ((ticks + 200 - ge->time) / 40) %
			levelInst->getAnim(set[anim])->frames;
	else if (set[E_ANIMSP])
		frame = (ticks / (set[E_ANIMSP] * 40)) %
			levelInst->getAnim(set[anim])->frames;
	else frame = (ticks / 20) % levelInst->getAnim(set[anim])->frames;

	dst.x = dstx = (x >> 10) - (localPlayer->viewX >> 10) +
		levelInst->getFrame(set[anim], frame)->x;

	dst.y = dsty = (y >> 10) - (localPlayer->viewY >> 10) +
		levelInst->getFrame(set[anim], frame)->y +
		levelInst->getAnim(set[anim])->y[frame] -
		levelInst->getFrame(set[anim], 0)->pixels->h;

	sprite = levelInst->getFrame(set[anim], frame)->pixels;

	if (ticks < flashTime) scalePalette(sprite, 0, 0);

	// Draw the event

	if (set[E_BEHAVIOUR] == 28) {

		dstx -= 10;
		dsty -= 32 - set[E_YAXIS];

		if (y > (gridY + 1) << 15) {

			if (anim == E_LEFTANIM)
				midpoint = (y - ((gridY + 1) << 15)) >> 10;
			else midpoint = set[E_MULTIPURPOSE] -
				((y - ((gridY + 1) << 15)) >> 10);

		} else midpoint = 0;

		if (midpoint < set[E_MULTIPURPOSE] / 2) {

			for (count = 0; count < set[E_MULTIPURPOSE]; count++) {

				dst.x = dstx += 8;

				if (midpoint == 0) dst.y = dsty;
				else if (count < midpoint)
					dst.y = ((dsty - midpoint) *
						(midpoint - count) / midpoint) +
						(dsty * count / midpoint);
				else
					dst.y = ((dsty - midpoint) * (count - midpoint) /
						(set[E_MULTIPURPOSE] - midpoint)) +
						(dsty * (set[E_MULTIPURPOSE] - count) /
						(set[E_MULTIPURPOSE] - midpoint));

				SDL_BlitSurface(sprite, NULL, screen, &dst);

			}

		} else {

			for (count = 0; count < set[E_MULTIPURPOSE]; count++) {

				dst.x = dstx += 8;

				if (midpoint == 0) dst.y = dsty;
				else if (count < midpoint)
					dst.y = ((dsty + midpoint - set[E_MULTIPURPOSE]) *
						(midpoint - count) / midpoint) +
						(dsty * count / midpoint);
				else
					dst.y = ((dsty + midpoint - set[E_MULTIPURPOSE]) *
						(count - midpoint) /
						(set[E_MULTIPURPOSE] - midpoint)) +
						(dsty * (set[E_MULTIPURPOSE] - count) /
						(set[E_MULTIPURPOSE] - midpoint));

				SDL_BlitSurface(sprite, NULL, screen, &dst);

			}

		}

	} else {

		SDL_BlitSurface(sprite, NULL, screen, &dst);

	}

	if (ticks < flashTime) restorePalette(sprite);

	// If the event has been destroyed, draw an explosion
	if (set[E_HITSTOKILL] &&
		((anim == E_LFINISHANIM) || (anim == E_RFINISHANIM))) {

		dst.x = dstx + levelInst->getFrame(121, frame)->x;
		dst.y = dsty + levelInst->getFrame(121, frame)->y +
			levelInst->getAnim(121)->y[frame];

		SDL_BlitSurface(levelInst->getFrame(121, frame)->pixels,
			NULL, screen, &dst);

	}


	if ((set[E_MODIFIER] == 8) && set[E_HITSTOKILL]) {

		// Draw boss energy bar

		count = ge->hits * 100 / set[E_HITSTOKILL];


		// Devan head

		sprite = levelInst->getSprite(225)->pixels;

		if (ticks < flashTime) scalePalette(sprite, 0, 0);

		dst.x = localPlayer->viewW - 44;
		dst.y = count + 23;
		SDL_BlitSurface(sprite, NULL, screen, &dst);

		if (ticks < flashTime) restorePalette(sprite);


		// Bar
		dst.x = localPlayer->viewW - 40;
		dst.y = count + 40;
		dst.w = 12;
		dst.h = 100 - count;
		if (ticks < flashTime) SDL_FillRect(screen, &dst, 0);
		else SDL_FillRect(screen, &dst, 32);

	}


	return;

}

