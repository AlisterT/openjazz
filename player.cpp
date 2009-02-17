
/*
 *
 * player.cpp
 *
 * Created on the 3rd of February 2009
 * Added parts of events.cpp and level.cpp on the 5th of February 2009
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

#include "OpenJazz.h"
#include <string.h>

Player::Player () {

	name = new char[5];
	strcpy(name, "JAZZ");

	score = 0;
	lives = 3;
	ammoType = -1;
	ammo[0] = 0;
	ammo[1] = 0;
	ammo[2] = 0;
	ammo[3] = 0;
	fireSpeed = 0;
	checkX = 0;
	checkY = 65;

	return;

}


Player::~Player () {

	delete[] name;

	return;

}


void Player::setAnim (int index, char anim) {

	anims[index] = anim;

	return;

}


void Player::setName (char * playerName) {

	delete[] name;
	name = new char[strlen(playerName) + 1];
	strcpy(name, playerName);

	return;

}


char * Player::getName () {

	return name;

}


void Player::reset () {

	facing = true;
	x = checkX << 15;
	y = checkY << 15;
	dx = 0;
	dy = 0;
	jumpHeight = 92 * F1;
	jumpY = 65 * F32;
	energy = 4;
	energyBar = 0;
	reaction = PR_NONE;
	reactionTime = PRT_NONE;
	floating = false;
	event = NULL;
	shield = 0;
	enemies = 0;
	items = 0;

	return;

}


void Player::setCheckpoint (unsigned char newX, unsigned char newY) {

	checkX = newX;
	checkY = newY;

	return;

}


void Player::setControl (int control, bool state) {

	pcontrols[control] = state;

	return;

}


void Player::addScore (int addedScore) {

	score += addedScore * 10;

	return;

}

int Player::getScore () {

	return score;

}

void Player::addCarrot () {

	if (energy < 4) energy++;

	return;

}


void Player::hit (int ticks) {

	if (reaction != PR_NONE) return;

	if (shield == 3) shield = 0;
	else if (shield) shield--;
	else energy--;

	if (energy) {

		reaction = PR_HURT;
		reactionTime = ticks + PRT_HURT;

		if (dx < 0) {

			dx = PXS_RUN;
			dy = PYS_JUMP;

		} else {

			dx = -PXS_RUN;
			dy = PYS_JUMP;

		}

	} else {

		kill(ticks);

	}

	return;

}


int Player::getEnergy () {

	return energy;

}


int Player::getEnergyBar () {

	if ((energyBar >> 10) < (energy << 4)) {

		if ((energy << 14) - energyBar < mspf * 40) energyBar = energy << 14;
		else energyBar += mspf * 40;

	} else if ((energyBar >> 10) > (energy << 4)) {

		if (energyBar - (energy << 14) < mspf * 40) energyBar = energy << 14;
		else energyBar -= mspf * 40;

	}

	return energyBar;

}


void Player::addLife () {

	if (lives < 99) lives++;

	return;

}


void Player::kill (int ticks) {

	if (reaction == PR_WON) return;

	lives--;

	reaction = PR_KILLED;
	reactionTime = ticks + PRT_KILLED;

	return;

}


int Player::getLives () {

	return lives;

}

void Player::addAmmo (int type, int amount) {

	if (!ammo[type]) ammoType = type;
	ammo[type] += amount;

	return;

}

void Player::changeAmmo () {

	ammoType = ((ammoType + 2) % 5) - 1;

	// If there is no ammo of this type, go to the next type that has ammo
	while ((ammoType > -1) && !ammo[ammoType])
		ammoType = ((ammoType + 2) % 5) - 1;

	return;

}


void Player::fire (int ticks) {

	// If a bullet has been fired too recently, do nothing
	if (ticks <= fireTime) return;

	// Create new bullet
	levelInst->firstBullet = new Bullet(this, false, ticks,
		levelInst->firstBullet);

	// Set when the next bullet can be fired
	if (fireSpeed) fireTime = ticks + (1000 / fireSpeed);
	else fireTime = 0x7FFFFFFF;

	// Remove the bullet from the arsenal
	if (ammoType != -1) ammo[ammoType]--;

	// If the current ammo has been exhausted, go to the previous type that
	// has ammo
	while ((ammoType > -1) && !ammo[ammoType]) ammoType--;

	return;

}


int Player::getAmmo (bool amount) {

	return amount? ammo[ammoType]: ammoType;

}


void Player::addRapidFire () {

	fireSpeed++;

	return;

}


void Player::addFeet () {

	jumpHeight += F16;

	return;

}


void Player::addStar (int ticks) {

	reaction = PR_INVINCIBLE;
	reactionTime = ticks + PRT_INVINCIBLE;

	return;

}

void Player::addShield (bool four) {

	if (four) shield = 6;
	else shield = 2;

	return;

}

void Player::win (int ticks) {

	reaction = PR_WON;
	reactionTime = ticks + PRT_WON;

	levelInst->win(ticks);

	return;

}


fixed Player::getX () {

	return x;

}


fixed Player::getY () {

	return y;

}


bool Player::isIn (fixed left, fixed top, fixed width, fixed height) {

	return (x + PXO_R >= left) && (x + PXO_L < left + width) && (y >= top) &&
		(y + PYO_TOP < top + height);

}


void Player::setPosition (fixed newX, fixed newY) {

	x = newX;
	y = newY;

	return;

}


void Player::setSpeed (fixed newDx, fixed newDy) {

	dx = newDx;
	dy = newDy;

	return;

}


void Player::setFloating (bool newFloating) {

	floating = newFloating;

	return;

}


bool Player::getFacing () {

	return facing;

}


void Player::floatUp (signed char *newEvent) {

	event = newEvent;

	if ((dy > 0) && levelInst->checkMaskDown(x + PXO_MID, y + F4))
		dy = event[E_MULTIPURPOSE] * -F40;

	if (dy > event[E_MULTIPURPOSE] * -F40)
		dy -= event[E_MULTIPURPOSE] * 320 * mspf;

	jumpY = y - (8 * F16);

	return;

}


void Player::belt (int speed) {

	if (speed < 0) dx += speed * 4 * mspf;
	else dx += speed * 40 * mspf;

	return;

}


void Player::setEvent (signed char *newEvent) {

	event = newEvent;

	if (event[E_MODIFIER] == 29) // Upwards spring
		jumpY = y + (event[E_MAGNITUDE] * (F20 + F1));

	return;

}


void Player::clearEvent (signed char *newEvent, unsigned char property) {

	// If the given property matches, clear the event

	if (event && (event[property] == newEvent[property])) event = NULL;

	return;

}


void Player::control (int ticks) {

	// Respond to controls, unless the player has been killed

	// If the player has been killed, drop but otherwise do not move
	if (!energy) {

		dx = 0;

		if (floating) dy = 0;
		else {

			dy += PYA_GRAVITY * mspf;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		return;

	}

	if (pcontrols[C_LEFT]) {

		// Wlak/run left

		if (dx > 0) dx -= PXA_REVERSE * mspf;
		if (dx > -PXS_WALK) dx -= PXA_WALK * mspf;
		if (dx > -PXS_RUN) dx -= PXA_RUN * mspf;

		facing = false;

	} else if (pcontrols[C_RIGHT]) {

		// Walk/run right

		if (dx < 0) dx += PXA_REVERSE * mspf;
		if (dx < PXS_WALK) dx += PXA_WALK * mspf;
		if (dx < PXS_RUN) dx += PXA_RUN * mspf;

		facing = true;

	} else {

		// Slow down

		if (dx > 0) {

			if (dx < PXA_STOP * mspf) dx = 0;
			else dx -= PXA_STOP * mspf;

		}

		if (dx < 0) {

			if (dx > -PXA_STOP * mspf) dx = 0;
			else dx += PXA_STOP * mspf;

		}

	}

	if (dx < -PXS_RUN) dx = -PXS_RUN;
	if (dx > PXS_RUN) dx = PXS_RUN;


	if (floating) {

		if (pcontrols[C_UP]) {

			// Fly upwards

			if (dy > 0) dy -= PXA_REVERSE * mspf;
			if (dy > -PXS_WALK) dy -= PXA_WALK * mspf;
			if (dy > -PXS_RUN) dy -= PXA_RUN * mspf;

		} else if (pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += PXA_REVERSE * mspf;
			if (dy < PXS_WALK) dy += PXA_WALK * mspf;
			if (dy < PXS_RUN) dy += PXA_RUN * mspf;

		} else {

			// Slow down

			if (dy > 0) {

				if (dy < PXA_STOP * mspf) dy = 0;
				else dy -= PXA_STOP * mspf;

			}

			if (dy < 0) {

				if (dy > -PXA_STOP * mspf) dy = 0;
				else dy += PXA_STOP * mspf;

			}

		}

		if (event) {

			if (event[E_MODIFIER] == 29) dy = event[E_MULTIPURPOSE] * -F20;
			else if (event[E_BEHAVIOUR] == 25) dy = PYS_JUMP;

		}

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		if (dy > PXS_RUN) dy = PXS_RUN;

	} else if (y > levelInst->getWaterLevel(ticks)) {

		if (pcontrols[C_JUMP]) {

			// Swim upwards

			if (dy > 0) dy -= PXA_REVERSE * mspf;
			if (dy > -PXS_WALK) dy -= PXA_WALK * mspf;
			if (dy > -PXS_RUN) dy -= PXA_RUN * mspf;

			// Prepare to jump upon leaving the water

			if (!levelInst->checkMask(x + PXO_MID, y - F36)) {

				jumpY = y -
					(jumpHeight + (4928 * F1 * mspf / (1000 + (136 * mspf))));

				if (dx < 0) jumpY += dx / 40;
				else if (dx > 0) jumpY -= dx / 40;

				event = NULL;

			}

		} else if (pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += PXA_REVERSE * mspf;
			if (dy < PXS_WALK) dy += PXA_WALK * mspf;
			if (dy < PXS_RUN) dy += PXA_RUN * mspf;

		} else {

			// Sink

			dy += PYA_SINK * mspf;
			if (dy > PYS_SINK) dy = PYS_SINK;

		}

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		if (dy > PXS_RUN) dy = PXS_RUN;

	} else {

		// If jumping, rise
		if (y > jumpY) dy = (jumpY - ((2 * F32) + y)) * 4;

		if ((event && ((event[E_MODIFIER] == 6) ||
			(event[E_BEHAVIOUR] == 28))) ||
			levelInst->checkMaskDown(x + PXO_ML, y + F8) ||
			levelInst->checkMaskDown(x + PXO_MID, y + F8) ||
			levelInst->checkMaskDown(x + PXO_MR, y + F8)) {

			// Mask/platform/bridge below player

			if (pcontrols[C_JUMP] &&
				!levelInst->checkMask(x + PXO_MID, y - F36)) {

				// Jump

				jumpY = y -
					(jumpHeight + (4928 * F1 * mspf / (1000 + (136 * mspf))));

				if (dx < 0) jumpY += dx / 40;
				else if (dx > 0) jumpY -= dx / 40;

				event = NULL;

			}

			if (!lookTime) {

				// If requested, look up or down
				if (pcontrols[C_UP]) lookTime = -ticks;
				else if (pcontrols[C_DOWN]) lookTime = ticks;

			}

		} else {

			// No mask/platform/bridge below player
			// Cannot look up or down
			lookTime = 0;

		}

		// Stop jumping
		if (!pcontrols[C_JUMP] &&
			(!event || ((event[E_MODIFIER] != 29) &&
			(event[E_BEHAVIOUR] != 25)))) jumpY = 65 * F32;

		// Stop looking
		if (!pcontrols[C_UP] && !pcontrols[C_DOWN]) lookTime = 0;

		// Fall under gravity
		dy += PYA_GRAVITY * mspf;
		if (dy > PYS_FALL) dy = PYS_FALL;

	}

	// If there is an obstacle above and the player is not floating up, stop
	// rising
	if (levelInst->checkMask(x + PXO_MID, y + PYO_TOP - F4) && (jumpY < y) &&
		(!event || event[E_BEHAVIOUR] != 25)) {

		jumpY = 65 * F32;

		if (event && (event[E_MODIFIER] != 6) && (event[E_BEHAVIOUR] != 28))
			event = NULL;

	}

	// If jump completed, stop rising
	if (y <= jumpY) {

		jumpY = 65 * F32;

		if (event && (event[E_MODIFIER] != 6) && (event[E_BEHAVIOUR] != 28))
			event = NULL;

	}

	// Handle firing
	if (pcontrols[C_FIRE]) fire(ticks);
	else fireTime = 0;

	return;

}


void Player::move (int ticks) {

	Bullet *bul, *prevBul;
	fixed pdx, pdy;
	int count;

	// Apply as much of the trajectory as possible, without going into the
	// scenery

	pdx = (dx * mspf) >> 10;
	pdy = (dy * mspf) >> 10;


	// First for the vertical component of the trajectory

	if (pdy >= 0) count = pdy >> 12;
	else count = -((-pdy) >> 12);

	while (count > 0) {

		if (levelInst->checkMaskDown(x + PXO_ML, y + F4) ||
			levelInst->checkMaskDown(x + PXO_MID, y + F4) ||
			levelInst->checkMaskDown(x + PXO_MR, y + F4)) break;

		y += F4;
		count--;

	}


	while (count < 0) {

		if (levelInst->checkMask(x + PXO_MID, y + PYO_TOP - F4)) break;

		y -= F4;
		count++;

	}

	if (pdy >= 0) pdy &= 4095;
	else pdy = -((-pdy) & 4095);

	if (((pdy > 0) && !(levelInst->checkMaskDown(x + PXO_ML, y + pdy) ||
		levelInst->checkMaskDown(x + PXO_MID, y + pdy) ||
		levelInst->checkMaskDown(x + PXO_MR, y + pdy))) || ((pdy < 0) &&
		!levelInst->checkMask(x + PXO_MID, y + pdy + PYO_TOP))) y += pdy;


	// Then for the horizontal component of the trajectory

	if (pdx >= 0) count = pdx >> 12;
	else count = -((-pdx) >> 12);

	while (count < 0) {

		if (levelInst->checkMask(x + PXO_L - F4, y + PYO_MID)) break;

		x -= F4;
		count++;

	}

	while (count > 0) {

		if (levelInst->checkMask(x + PXO_R + F4, y + PYO_MID)) break;

		x += F4;
		count--;

	}

	if (pdx >= 0) pdx &= 4095;
	else pdx = -((-pdx) & 4095);

	if (((pdx < 0) && !levelInst->checkMask(x + PXO_L + pdx, y + PYO_MID)) ||
		((pdx > 0) && !levelInst->checkMask(x + PXO_R + pdx, y + PYO_MID)))
		x += pdx;

	// If on an uphill slope, push the player upwards
	if (pdx < 0)
		while (levelInst->checkMask(x + PXO_ML, y) &&
			!levelInst->checkMask(x + PXO_ML, y + PYO_TOP)) y -= F1;
	else
		while (levelInst->checkMask(x + PXO_MR, y) &&
			!levelInst->checkMask(x + PXO_MR, y + PYO_TOP)) y -= F1;


	// If using a float up event and have hit a ceiling, ignore event
	if (event && (event[E_BEHAVIOUR] == 25) &&
		levelInst->checkMask(x + PXO_MID, y + PYO_TOP - F4)) {

		jumpY = 65 * F32;
		event = NULL;

	}


	// Handle spikes
	if (
		/* Above */
		((levelInst->getGrid((x + PXO_MID) >> 15,
		(y + PYO_TOP) >> 15)->event == 126) &&
		levelInst->checkMask(x + PXO_MID, y + PYO_TOP - F4)) ||

		/* Below */
		((levelInst->getGrid((x + PXO_MID) >> 15, y >> 15)->event == 126) &&
		levelInst->checkMaskDown(x + PXO_MID, y + F4)) ||

		/* To left*/
		((levelInst->getGrid((x + PXO_L) >> 15,
		(y + PYO_MID) >> 15)->event == 126) &&
		levelInst->checkMask(x + PXO_L - F4, y + PYO_MID)) ||

		/* To right*/
		((levelInst->getGrid((x + PXO_R) >> 15,
		(y + PYO_MID) >> 15)->event == 126) &&
		levelInst->checkMask(x + PXO_R + F4, y + PYO_MID)))

		hit(ticks);


	// Deal with bullet collisions

	bul = levelInst->firstBullet;
	prevBul = NULL;

	while (bul) {

		if ((bul->getSource() != this) &&
			bul->isIn(x + PXO_L, y + PYO_TOP, PXO_R - PXO_L, -PYO_TOP)) {

			hit(ticks);

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

	return;

}


void Player::view (int ticks) {

	int oldViewX, oldViewY, speed;

	// Calculate viewport

	// Record old viewport position for applying lag
	oldViewX = viewX;
	oldViewY = viewY;

	// Can we see below the panel?
	viewW = screenW;
	if (viewW > panel->w) viewH = screenH;
	else viewH = screenH - 33;

	// Find new position
	if (reaction != PR_WON) {

		viewX = x + F8 - (viewW << 9);

		if (!lookTime || (ticks < 1000 + lookTime) ||
			(ticks < 1000 - lookTime)) {

			viewY = y - F24 - (viewH << 9);

		} else if (lookTime > 0) {

			if (ticks < 2000 + lookTime)
				viewY = y - (F24 - (64 * (ticks - (1000 + lookTime)))) -
				(viewH << 9);
			else viewY = y - (F24 - F64) - (viewH << 9);

		} else {

			if (ticks < 2000 - lookTime)
				viewY = y - (F24 + (64 * (ticks - (1000 - lookTime)))) -
					(viewH << 9);
			else viewY = y - (F24 + F64) - (viewH << 9);

		}

	} else {

		if (checkX << 15 > viewX + (viewW << 9) + (160 * mspf))
			viewX += 160 * mspf;
		else if (checkX << 15 < viewX + (viewW << 9) - (160 * mspf))
			viewX -= 160 * mspf;

		if (checkY << 15 > viewY + (viewH << 9) + (160 * mspf))
			viewY += 160 * mspf;
		else if (checkY << 15 < viewY + (viewH << 9) - (160 * mspf))
			viewY -= 160 * mspf;

	}


	// Apply lag proportional to player "speed"
	speed = ((dx >= 0? dx: -dx) + (dy >= 0? dy: -dy)) >> 15;

	if (mspf < speed) {

		viewX = ((oldViewX * (speed - mspf)) + (viewX * mspf)) / speed;
		viewY = ((oldViewY * (speed - mspf)) + (viewY * mspf)) / speed;

	}


	// Ensure the new viewport is within the level
	if (viewX < 0) viewX = 0;
	if ((viewX >> 10) + viewW >= LW * TW) viewX = ((LW * TW) - viewW) << 10;
	if (viewY < 0) viewY = 0;
	if ((viewY >> 10) + viewH >= LH * TH)	viewY = ((LH * TH) - viewH) << 10;

	return;

}

void Player::draw (int ticks) {

	SDL_Rect dst;
	int anim, frame;

	// The current frame for animations
	if (energy) frame = ticks / 75;
	else {

		frame = (ticks + PRT_KILLED - reactionTime) / 75;
		if (frame >= levelInst->getAnim(anims[PA_LDIE])->frames)
			frame = levelInst->getAnim(anims[PA_LDIE])->frames - 1;

	}

	// Choose player animation

	if (reaction == PR_KILLED) anim = anims[facing? PA_RDIE: PA_LDIE];

	else if ((reaction == PR_HURT) && (reactionTime - ticks > 800))
		anim = anims[facing? PA_RHURT: PA_LHURT];

	else if (y > levelInst->getWaterLevel(ticks))
		anim = anims[facing? PA_RSWIM: PA_LSWIM];

	else if (floating) anim = anims[facing? PA_RBOARD: PA_LBOARD];

	else if (dy >= 0) {

		if ((event && ((event[E_MODIFIER] == 6) ||
		    (event[E_BEHAVIOUR] == 28))) ||
		    levelInst->checkMaskDown(x + PXO_ML, y + F4) ||
		    levelInst->checkMaskDown(x + PXO_MID, y + F4) ||
		    levelInst->checkMaskDown(x + PXO_MR, y + F4) ||
		    levelInst->checkMaskDown(x + PXO_ML, y + F12) ||
		    levelInst->checkMaskDown(x + PXO_MID, y + F12) ||
		    levelInst->checkMaskDown(x + PXO_MR, y + F12)   ) {

			if (dx) {

				if (dx <= -PXS_RUN) anim = anims[PA_LRUN];
				else if (dx >= PXS_RUN) anim = anims[PA_RRUN];
				else if ((dx < 0) && facing) anim = anims[PA_LSTOP];
				else if ((dx > 0) && !facing) anim = anims[PA_RSTOP];
				else anim = anims[facing? PA_RWALK: PA_LWALK];

			} else {

				if (!levelInst->checkMaskDown(x + F12, y + F12) &&
					!levelInst->checkMaskDown(x + F8, y + F8) &&
					(!event || ((event[E_MODIFIER] != 6) &&
					(event[E_BEHAVIOUR] != 28))))
					anim = anims[PA_LEDGE];

				else if (!levelInst->checkMaskDown(x + F20, y + F12) &&
					!levelInst->checkMaskDown(x + F24, y + F8) &&
					(!event || ((event[E_MODIFIER] != 6) &&
					(event[E_BEHAVIOUR] != 28))))
					anim = anims[PA_REDGE];

				else if (pcontrols[C_FIRE])
					anim = anims[facing? PA_RSHOOT: PA_LSHOOT];

				else if ((lookTime < 0) && (ticks > 1000 - lookTime))
					anim = anims[PA_LOOKUP];

				else if (lookTime > 0) {

					if (ticks < 1000 + lookTime)
						anim = anims[facing? PA_RCROUCH: PA_LCROUCH];
					else anim = anims[PA_LOOKDOWN];

				} else anim = anims[facing? PA_RSTAND: PA_LSTAND];

			}

		} else anim = anims[facing? PA_RFALL: PA_LFALL];

	} else if (event && (event[E_MODIFIER] == 29))
		anim = anims[facing? PA_RSPRING: PA_LSPRING];
    
    else anim = anims[facing? PA_RJUMP: PA_LJUMP];


    // Show the player

    // Flash red if hurt
    if ((reaction == PR_HURT) && (!((ticks / 30) & 3)))
    	scalePalette(levelInst->getFrame(anim, frame)->pixels, 0, 36);

    dst.x = ((x - viewX) >> 10) + levelInst->getFrame(anim, frame)->x;

    dst.y = ((y + F4 - viewY) >> 10) +
    	levelInst->getFrame(anim, frame)->y +
    	levelInst->getAnim(anim)->y[frame % levelInst->getAnim(anim)->frames] -
    	levelInst->getFrame(anim, 0)->pixels->h;

    SDL_BlitSurface(levelInst->getFrame(anim, frame)->pixels, NULL, screen,
    	&dst);

   	// Remove red flash
    if ((reaction == PR_HURT) && (!((ticks / 30) & 3)))
    	restorePalette(levelInst->getFrame(anim, frame)->pixels);

	// Show invincibility stars

	if (reaction == PR_INVINCIBLE) {

		dst.x = (x + PXO_L - viewX) >> 10;
		dst.y = (y - F32 - viewY) >> 10;
		SDL_BlitSurface(levelInst->getFrame(122, frame)->pixels, NULL, screen,
			&dst);

		dst.x = (x + PXO_MID - viewX) >> 10;
		dst.y = (y - F32 - viewY) >> 10;
		SDL_BlitSurface(levelInst->getFrame(122, frame)->pixels, NULL, screen,
			&dst);

		dst.x = (x + PXO_L - viewX) >> 10;
		dst.y = (y - F16 - viewY) >> 10;
		SDL_BlitSurface(levelInst->getFrame(122, frame)->pixels, NULL, screen,
			&dst);

		dst.x = (x + PXO_MID - viewX) >> 10;
		dst.y = (y - F16 - viewY) >> 10;
		SDL_BlitSurface(levelInst->getFrame(122, frame)->pixels, NULL, screen,
			&dst);

	}


	return;

}


int Player::reacted (int ticks) {

	int oldReaction;

	if ((reaction != PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = PR_NONE;

		return oldReaction;

	}

	return PR_NONE;

}


