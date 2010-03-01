
/*
 *
 * playerframe.cpp
 *
 * 18th July 2009: Created playerframe.cpp from parts of player.cpp
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
 * Provides the once-per-frame functions of players.
 *
 */


#include "bird.h"
#include "player.h"

#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "level/bullet.h"
#include "level/event/event.h"
#include "level/level.h"

#include <math.h>


void Player::control (unsigned int ticks, int msps) {

	// Respond to controls, unless the player has been killed

	// If the player has been killed, drop but otherwise do not move
	if (!energy) {

		dx = 0;

		if (floating) dy = 0;
		else {

			dy += PYA_GRAVITY * msps;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		return;

	}

	if (pcontrols[C_RIGHT]) {

		// Walk/run right

		if (dx < 0) dx += PXA_REVERSE * msps;
		else if (dx < PXS_WALK) dx += PXA_WALK * msps;
		else if (dx < PXS_RUN) dx += PXA_RUN * msps;

		facing = true;

	} else if (pcontrols[C_LEFT]) {

		// Walk/run left

		if (dx > 0) dx -= PXA_REVERSE * msps;
		else if (dx > -PXS_WALK) dx -= PXA_WALK * msps;
		else if (dx > -PXS_RUN) dx -= PXA_RUN * msps;

		facing = false;

	} else {

		// Slow down

		if (dx > 0) {

			if (dx < PXA_STOP * msps) dx = 0;
			else dx -= PXA_STOP * msps;

		}

		if (dx < 0) {

			if (dx > -PXA_STOP * msps) dx = 0;
			else dx += PXA_STOP * msps;

		}

	}

	if (dx < -PXS_RUN) dx = -PXS_RUN;
	if (dx > PXS_RUN) dx = PXS_RUN;


	if (floating) {

		if (pcontrols[C_UP]) {

			// Fly upwards

			if (dy > 0) dy -= PXA_REVERSE * msps;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * msps;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * msps;

		} else if (pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += PXA_REVERSE * msps;
			else if (dy < PXS_WALK) dy += PXA_WALK * msps;
			else if (dy < PXS_RUN) dy += PXA_RUN * msps;

		} else {

			// Slow down

			if (dy > 0) {

				if (dy < PXA_STOP * msps) dy = 0;
				else dy -= PXA_STOP * msps;

			}

			if (dy < 0) {

				if (dy > -PXA_STOP * msps) dy = 0;
				else dy += PXA_STOP * msps;

			}

		}

		if (event) {

			if (event[E_MODIFIER] == 29) dy = event[E_MULTIPURPOSE] * -F20;
			else if (event[E_BEHAVIOUR] == 25) dy = PYS_JUMP;

		}

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		if (dy > PXS_RUN) dy = PXS_RUN;

	} else if (y + PYO_MID > level->getWaterLevel()) {

		if (pcontrols[C_JUMP]) {

			// Swim upwards

			if (dy > 0) dy -= PXA_REVERSE * msps;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * msps;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * msps;

			// Prepare to jump upon leaving the water

			if (!level->checkMask(x + PXO_MID, y - F36)) {

				jumpY = y - jumpHeight;

				if (dx < 0) jumpY += dx >> 4;
				else if (dx > 0) jumpY -= dx >> 4;

				event = NULL;

			}

		} else if (pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += PXA_REVERSE * msps;
			else if (dy < PXS_WALK) dy += PXA_WALK * msps;
			else if (dy < PXS_RUN) dy += PXA_RUN * msps;

		} else {

			// Sink

			dy += PYA_SINK * msps;
			if (dy > PYS_SINK) dy = PYS_SINK;

		}

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		if (dy > PXS_RUN) dy = PXS_RUN;

	} else {

		if ((event && ((event[E_MODIFIER] == 6) ||
			(event[E_BEHAVIOUR] == 28))) ||
			level->checkMaskDown(x + PXO_ML, y + F2) ||
			level->checkMaskDown(x + PXO_MID, y + F2) ||
			level->checkMaskDown(x + PXO_MR, y + F2)) {

			// Mask/platform/bridge below player

			if (pcontrols[C_JUMP] && !level->checkMask(x + PXO_MID, y - F36)) {

				// Jump

				jumpY = y - jumpHeight;

				// Increase jump height if walking/running
				if (dx < 0) jumpY += dx >> 3;
				else if (dx > 0) jumpY -= dx >> 3;

				event = NULL;

				playSound(S_JUMPA);

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
			(event[E_BEHAVIOUR] != 25)))) jumpY = TTOF(LH);

		// If jumping, rise
		if (y >= jumpY) {

			dy = (jumpY - y - F64) * 4;

			// Spring speed limit
			if (event && (event[E_MODIFIER] == 29)) {

				if ((event[E_MULTIPURPOSE] == 0) && (dy < PYS_JUMP))
					dy = PYS_JUMP;

				if ((event[E_MULTIPURPOSE] > 0) && (dy < event[E_MULTIPURPOSE] * -F20))
					dy = event[E_MULTIPURPOSE] * -F20;

			}

			// Avoid jumping too fast, unless caused by an event
			if (!event && (dy < PYS_JUMP)) dy = PYS_JUMP;

		} else {

			// Fall under gravity
			dy += PYA_GRAVITY * msps;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		// Stop looking
		if (!pcontrols[C_UP] && (lookTime < 0)) lookTime = 0;
		if (!pcontrols[C_DOWN] && (lookTime > 0)) lookTime = 0;

	}

	// If there is an obstacle above and the player is not floating up, stop
	// rising
	if (level->checkMask(x + PXO_MID, y + PYO_TOP - F4) && (jumpY < y) &&
		(!event || event[E_BEHAVIOUR] != 25)) {

		jumpY = TTOF(LH);
		if (dy < 0) dy = 0;

		if (event && (event[E_MODIFIER] != 6) && (event[E_BEHAVIOUR] != 28))
			event = NULL;

	}

	// If jump completed, stop rising
	if (y <= jumpY) {

		jumpY = TTOF(LH);

		if (event && (event[E_MODIFIER] != 6) && (event[E_BEHAVIOUR] != 28))
			event = NULL;

	}


	// Handle firing
	if (pcontrols[C_FIRE]) {

		if (ticks > fireTime) {

			// Create new bullet
			level->firstBullet = new Bullet(this, false, ticks);

			// Set when the next bullet can be fired
			if (fireSpeed) fireTime = ticks + (1000 / fireSpeed);
			else fireTime = 0x7FFFFFFF;

			// Remove the bullet from the arsenal
			if (ammoType != -1) ammo[ammoType]--;

			/* If the current ammo type has been exhausted, use the previous
			non-exhausted ammo type */
			while ((ammoType > -1) && !ammo[ammoType]) ammoType--;

		}

	} else fireTime = 0;


	// Check for a change in ammo
	if (pcontrols[C_CHANGE]) {

		if (this == localPlayer) controls.release(C_CHANGE);

		ammoType = ((ammoType + 2) % 5) - 1;

		// If there is no ammo of this type, go to the next type that has ammo
		while ((ammoType > -1) && !ammo[ammoType])
			ammoType = ((ammoType + 2) % 5) - 1;

	}


	// Deal with the bird

	if (bird) {

		if (bird->step(ticks, msps)) {

			delete bird;
			bird = NULL;

		}

	}


	return;

}


void Player::move (unsigned int ticks, int msps) {

	fixed pdx, pdy;
	int count;

	if (warpTime && (ticks > warpTime)) {

		x = TTOF(warpX);
		y = TTOF(warpY + 1);
		warpTime = 0;

	}

	// Apply as much of the trajectory as possible, without going into the
	// scenery

	if (fastFeetTime > ticks) {

		pdx = (dx * msps * 3) >> 11;
		pdy = (dy * msps * 3) >> 11;

	} else {

		pdx = (dx * msps) >> 10;
		pdy = (dy * msps) >> 10;

	}

	// First for the vertical component of the trajectory

	if (pdy < 0) {

		// Moving up

		count = (-pdy) >> 12;

		while (count > 0) {

			if (level->checkMask(x + PXO_MID, y + PYO_TOP - F4)) break;

			y -= F4;
			count--;

		}

		pdy = (-pdy) & 4095;

		if (!level->checkMask(x + PXO_MID, y + PYO_TOP - pdy)) y -= pdy;
		else y &= ~4095;

	} else if (pdy > 0) {

		// Moving down

		count = pdy >> 12;

		while (count > 0) {

			if (level->checkMaskDown(x + PXO_ML, y + F4) ||
				level->checkMaskDown(x + PXO_MID, y + F4) ||
				level->checkMaskDown(x + PXO_MR, y + F4)) break;

			y += F4;
			count--;

		}

		pdy &= 4095;

		if (!(level->checkMaskDown(x + PXO_ML, y + pdy) ||
			level->checkMaskDown(x + PXO_MID, y + pdy) ||
			level->checkMaskDown(x + PXO_MR, y + pdy))) y += pdy;
		else y |= 4095;

	}



	// Then for the horizontal component of the trajectory

	if (pdx < 0) {

		// Moving left

		count = (-pdx) >> 12;

		while (count > 0) {

			// If there is an obstacle, stop
			if (level->checkMask(x + PXO_L - F4, y + PYO_MID)) break;

			x -= F4;
			count--;

			// If on an uphill slope, push the player upwards
			if (level->checkMask(x + PXO_ML, y) &&
				!level->checkMask(x + PXO_ML, y - F4)) y -= F4;

		}

		pdx = (-pdx) & 4095;

		if (!level->checkMask(x + PXO_L - pdx, y + PYO_MID)) x -= pdx;
		else x &= ~4095;

		// If on an uphill slope, push the player upwards
		while (level->checkMask(x + PXO_ML, y) &&
			!level->checkMask(x + PXO_ML, y - F4)) y -= F1;

	} else if (pdx > 0) {

		// Moving right

		count = pdx >> 12;

		while (count > 0) {

			// If there is an obstacle, stop
			if (level->checkMask(x + PXO_R + F4, y + PYO_MID)) break;

			x += F4;
			count--;

			// If on an uphill slope, push the player upwards
			if (level->checkMask(x + PXO_MR, y) &&
				!level->checkMask(x + PXO_MR, y - F4)) y -= F4;

		}

		pdx &= 4095;

		if (!level->checkMask(x + PXO_R + pdx, y + PYO_MID)) x += pdx;
		else x |= 4095;

		// If on an uphill slope, push the player upwards
		while (level->checkMask(x + PXO_MR, y) &&
			!level->checkMask(x + PXO_MR, y - F4)) y -= F1;

	}


	// If using a float up event and have hit a ceiling, ignore event
	if (event && (event[E_BEHAVIOUR] == 25) &&
		level->checkMask(x + PXO_MID, y + PYO_TOP - F4)) {

		jumpY = TTOF(LH);
		event = NULL;

	}


	if (level->getStage() == LS_END) return;


	// If the player has hit the bottom of the level, kill
	if (y + F4 > TTOF(LH)) kill(NULL, ticks);


	// Handle spikes
	if (level->checkSpikes(x + PXO_MID, y + PYO_TOP - F4) ||
		level->checkSpikes(x + PXO_MID, y + F4) ||
		level->checkSpikes(x + PXO_L - F4, y + PYO_MID) ||
		level->checkSpikes(x + PXO_R + F4, y + PYO_MID)) hit(NULL, ticks);


	return;

}


void Player::view (unsigned int ticks, int mspf) {

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

	viewX = x + F8 - (viewW << 9);
	viewY = y - F24 - (viewH << 9);

	if ((lookTime > 0) && ((int)ticks > 1000 + lookTime)) {

		// Look down
		if ((int)ticks < 2000 + lookTime) viewY += 64 * (ticks - (1000 + lookTime));
		else viewY += F64;

	} else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime)) {

		// Look up
		if ((int)ticks < 2000 - lookTime) viewY -= 64 * (ticks - (1000 - lookTime));
		else viewY -= F64;

	}


	// Apply lag proportional to player "speed"
	speed = ((dx >= 0? dx: -dx) + (dy >= 0? dy: -dy)) >> 14;

	if (mspf < speed) {

		viewX = ((oldViewX * (speed - mspf)) + (viewX * mspf)) / speed;
		viewY = ((oldViewY * (speed - mspf)) + (viewY * mspf)) / speed;

	}


	return;

}

void Player::draw (unsigned int ticks, int change) {

	Anim *an;
	int anim, frame;
	fixed drawX, drawY;
	fixed xOffset, yOffset;

	// The current frame for animations
	if (reaction == PR_KILLED) frame = (ticks + PRT_KILLED - reactionTime) / 75;
	else frame = ticks / 75;


	// Get position

	drawX = getDrawX(change);
	drawY = getDrawY(change);


	// Choose player animation

	if (reaction == PR_KILLED) anim = anims[facing? PA_RDIE: PA_LDIE];

	else if ((reaction == PR_HURT) &&
		(reactionTime - ticks > PRT_HURT - PRT_HURTANIM))
		anim = anims[facing? PA_RHURT: PA_LHURT];

	else if (y + PYO_MID > level->getWaterLevel())
		anim = anims[facing? PA_RSWIM: PA_LSWIM];

	else if (floating) anim = anims[facing? PA_RBOARD: PA_LBOARD];

	else if (dy >= 0) {

		if ((event && ((event[E_MODIFIER] == 6) ||
			(event[E_BEHAVIOUR] == 28))) ||
			level->checkMaskDown(x + PXO_ML, y + F2) ||
			level->checkMaskDown(x + PXO_MID, y + F2) ||
			level->checkMaskDown(x + PXO_MR, y + F2) ||
			level->checkMaskDown(x + PXO_ML, y + F8) ||
			level->checkMaskDown(x + PXO_MID, y + F8) ||
			level->checkMaskDown(x + PXO_MR, y + F8)) {

			drawX = x;
			drawY = y;

			if (dx) {

				if (dx <= -PXS_RUN) anim = anims[PA_LRUN];
				else if (dx >= PXS_RUN) anim = anims[PA_RRUN];
				else if ((dx < 0) && facing) anim = anims[PA_LSTOP];
				else if ((dx > 0) && !facing) anim = anims[PA_RSTOP];
				else anim = anims[facing? PA_RWALK: PA_LWALK];

			} else {

				if (!level->checkMaskDown(x + PXO_ML, y + F12) &&
					!level->checkMaskDown(x + PXO_L, y + F2) &&
					(!event || ((event[E_MODIFIER] != 6) &&
					(event[E_BEHAVIOUR] != 28))))
					anim = anims[PA_LEDGE];

				else if (!level->checkMaskDown(x + PXO_MR, y + F12) &&
					!level->checkMaskDown(x + PXO_R, y + F2) &&
					(!event || ((event[E_MODIFIER] != 6) &&
					(event[E_BEHAVIOUR] != 28))))
					anim = anims[PA_REDGE];

				else if (pcontrols[C_FIRE])
					anim = anims[facing? PA_RSHOOT: PA_LSHOOT];

				else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime))
					anim = anims[PA_LOOKUP];

				else if (lookTime > 0) {

					if ((int)ticks < 1000 + lookTime)
						anim = anims[facing? PA_RCROUCH: PA_LCROUCH];
					else anim = anims[PA_LOOKDOWN];

				} else anim = anims[facing? PA_RSTAND: PA_LSTAND];

			}

		} else anim = anims[facing? PA_RFALL: PA_LFALL];

	} else if (event && (event[E_MODIFIER] == 29))
		anim = anims[facing? PA_RSPRING: PA_LSPRING];

	else anim = anims[facing? PA_RJUMP: PA_LJUMP];


	// Choose sprite

	an = level->getAnim(anim);
	an->setFrame(frame, reaction != PR_KILLED);


	// Show the player

	// Flash red if hurt, otherwise use player colour
	if ((reaction == PR_HURT) && (!((ticks / 30) & 3)))
		an->flashPalette(36);

	else {

		an->setPalette(palette, 23, 41);
		an->setPalette(palette, 88, 8);

	}


	// Draw "motion blur"
	if (fastFeetTime > ticks) an->draw(drawX - (dx >> 6), drawY);

	// Draw player
	an->draw(drawX, drawY);


	// Remove red flash or player colour from sprite
	an->restorePalette();


	// Uncomment the following to see the area of the player
	/*drawRect(FTOI(getDrawX(change) + PXO_L - viewX),
		FTOI(getDrawY(change) + PYO_TOP - viewY),
		FTOI(PXO_R - PXO_L),
		FTOI(-PYO_TOP), 89);
	drawRect(FTOI(getDrawX(change) + PXO_ML - viewX),
		FTOI(getDrawY(change) + PYO_TOP - viewY),
		FTOI(PXO_MR - PXO_ML),
		FTOI(-PYO_TOP), 88);*/


	if (reaction == PR_INVINCIBLE) {

		// Show invincibility stars

		xOffset = (int)(sin(ticks / 100.0f) * F12);
		yOffset = (int)(cos(ticks / 100.0f) * F12);

		an = level->getMiscAnim(0);

		an->setFrame(frame, true);
		an->draw(drawX + PXO_MID + xOffset, drawY + PYO_MID + yOffset);

		an->setFrame(frame + 1, true);
		an->draw(drawX + PXO_MID - xOffset, drawY + PYO_MID - yOffset);

		an->setFrame(frame + 2, true);
		an->draw(drawX + PXO_MID + yOffset, drawY + PYO_MID + xOffset);

		an->setFrame(frame + 3, true);
		an->draw(drawX + PXO_MID - yOffset, drawY + PYO_MID - xOffset);

	} else if (shield > 2) {

		// Show the 4-hit shield

		xOffset = (int)(cos(ticks / 200.0f) * F20);
		yOffset = (int)(sin(ticks / 200.0f) * F20);

		an = level->getAnim(59);

		an->draw(drawX + xOffset, drawY + PYO_TOP + yOffset);

		if (shield > 3) an->draw(drawX - xOffset, drawY + PYO_TOP - yOffset);

		if (shield > 4) an->draw(drawX + yOffset, drawY + PYO_TOP - xOffset);

		if (shield > 5) an->draw(drawX - yOffset, drawY + PYO_TOP + xOffset);

	} else if (shield) {

		// Show the 2-hit shield

		xOffset = (int)(cos(ticks / 200.0f) * F20);
		yOffset = (int)(sin(ticks / 200.0f) * F20);

		an = level->getAnim(50);

		an->draw(drawX + xOffset, drawY + yOffset + PYO_TOP);

		if (shield == 2) an->draw(drawX - xOffset, drawY + PYO_TOP - yOffset);

	}


	// Show the bird
	if (bird) bird->draw(ticks, change);


	// Show the player's name
	if (gameMode)
		panelBigFont->showString(name, FTOI(drawX - viewX),
			FTOI(drawY - F32 - F16 - viewY));

	return;

}


