
/**
 *
 * @file jj1levelplayerframe.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 1st January 2006: Created events.c from parts of level.c
 * 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 18th July 2009: Created playerframe.cpp from parts of player.cpp
 * 24th June 2010: Renamed playerframe.cpp to levelplayerframe.cpp
 * 29th June 2010: Created jj2levelplayerframe.cpp from parts of
 *                 levelplayerframe.cpp
 * 1st August 2012: Renamed levelplayerframe.cpp to jj1levelplayerframe.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Provides the once-per-frame functions of players in levels.
 *
 */


#include "../jj1bullet.h"
#include "../jj1event/jj1event.h"
#include "../jj1level.h"
#include "jj1bird.h"
#include "jj1levelplayer.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "util.h"


/**
 * Determine whether or not the area below the player is solid when travelling
 * downwards.
 *
 * @param yOffset Vertical offset of the mask values to check
 *
 * @return Solidity
 */
bool JJ1LevelPlayer::checkMaskDown (fixed yOffset) {

	return level->checkMaskDown(x + PXO_ML + F1, y + yOffset) ||
		level->checkMaskDown(x + PXO_MID, y + yOffset) ||
		level->checkMaskDown(x + PXO_MR - F1, y + yOffset);

}


/**
 * Determine whether or not the area above the player is solid when travelling
 * upwards.
 *
 * @param yOffset Vertical offset of the mask values to check
 *
 * @return Solidity
 */
bool JJ1LevelPlayer::checkMaskUp (fixed yOffset) {

	return level->checkMaskUp(x + PXO_ML + F1, y + yOffset) ||
		level->checkMaskUp(x + PXO_MID, y + yOffset) ||
		level->checkMaskUp(x + PXO_MR - F1, y + yOffset);

}


/**
 * Move the player to the ground's surface.
 */
void JJ1LevelPlayer::ground () {

	// If on an uphill slope, push the player upwards
	if (checkMaskUp(0) && !checkMaskUp(-F4)) y -= F4;

	// If on a downhill slope, push the player downwards
	if (!checkMaskUp(F4) && checkMaskUp(F8)) y += F4;

	return;

}


/**
 * Respond to controls, unless the player has been killed.
 *
 * @param ticks Time
 */
void JJ1LevelPlayer::control (unsigned int ticks) {

	fixed speed;
	bool platform;


	// If the player has been killed, drop but otherwise do not move
	if (!energy) {

		dx = 0;
		udx = 0;

		if (flying) dy = 0;
		else {

			if (dy < 0) dy += PYA_GRAVITY;
			else dy = PYS_FALL;

		}

		return;

	}

	if ((event == JJ1PE_FLOATH) || (event == JJ1PE_REPELH)) {

		speed = level->getEvent(eventX, eventY)->magnitude *
			((event == JJ1PE_FLOATH)? F10: F40);

		if (speed < -PXS_WALK || speed > PXS_WALK) {

			udx = speed;

		} else if ((speed < 0) && player->pcontrols[C_RIGHT]) {

			// Walk right, against the flow

			if (udx < speed) udx += PXA_REVERSE;
			else if (udx < speed + PXS_WALK) udx += PXA_WALK;

			if (udx > speed + PXS_WALK) udx = speed + PXS_WALK;

			facing = true;

		} else if ((speed > 0) && player->pcontrols[C_LEFT]) {

			// Walk left, against the flow

			if (udx > 0) udx -= PXA_REVERSE;
			else if (udx > speed - PXS_WALK) udx -= PXA_WALK;

			if (udx < speed - PXS_WALK) udx = speed - PXS_WALK;

			facing = false;

		} else {

			udx = speed;

		}

		event = JJ1PE_NONE;

	} else if (player->pcontrols[C_RIGHT]) {

		// Walk/run right

		if (udx < 0) udx = dx;

		if (udx < 0) udx += PXA_REVERSE;
		else if (udx < PXS_WALK) udx += PXA_WALK;
		else if (udx < PXS_RUN) udx += PXA_RUN;

		if (udx > PXS_RUN) udx = PXS_RUN;

		facing = true;

	} else if (player->pcontrols[C_LEFT]) {

		// Walk/run left

		if (udx > 0) udx = dx;

		if (udx > 0) udx -= PXA_REVERSE;
		else if (udx > -PXS_WALK) udx -= PXA_WALK;
		else if (udx > -PXS_RUN) udx -= PXA_RUN;

		if (udx < -PXS_RUN) udx = -PXS_RUN;

		facing = false;

	} else {

		// Slow down

		if (udx > 0) {

			if (udx < PXA_STOP) udx = 0;
			else udx -= PXA_STOP;

		} else if (udx < 0) {

			if (udx > -PXA_STOP) udx = 0;
			else udx += PXA_STOP;

		}

	}


	// Check for platform event, bridge or level mask below player
	platform = (event == JJ1PE_PLATFORM) ||
		checkMaskDown(F4) ||
		((dx > 0) && level->checkMaskDown(x + PXO_ML, y + F8)) ||
		((dx < 0) && level->checkMaskDown(x + PXO_MR, y + F8));


	if (flying) {

		if (player->pcontrols[C_UP]) {

			// Fly upwards

			if (dy > 0) dy -= PXA_REVERSE;
			else if (dy > -PXS_WALK) dy -= PXA_WALK;
			else if (dy > -PXS_RUN) dy -= PXA_RUN;

		} else if (player->pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += PXA_REVERSE;
			else if (dy < PXS_WALK) dy += PXA_WALK;
			else if (dy < PXS_RUN) dy += PXA_RUN;

		} else {

			// Slow down

			if (dy > 0) {

				if (dy < PXA_STOP) dy = 0;
				else dy -= PXA_STOP;

			} else if (dy < 0) {

				if (dy > -PXA_STOP) dy = 0;
				else dy += PXA_STOP;

			}

		}

		if (event == JJ1PE_SPRING) dy = level->getEvent(eventX, eventY)->multiA * -F20;
		else if (event == JJ1PE_FLOAT) dy = PYS_JUMP;

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		else if (dy > PXS_RUN) dy = PXS_RUN;

	} else if (y + PYO_MID > level->getWaterLevel() + F8) {

		if (player->pcontrols[C_SWIM]) {

			// Swim upwards

			if (dy > 0) dy -= PXA_REVERSE;
			else if (dy > -PXS_WALK) dy -= PXA_WALK;
			else if (dy > -PXS_RUN) dy -= PXA_RUN;

			// Prepare to jump upon leaving the water

			if (!level->checkMaskUp(x + PXO_MID, y - F36)) {

				targetY = y - jumpHeight;

				if (dx < 0) targetY += dx >> 4;
				else if (dx > 0) targetY -= dx >> 4;

				event = JJ1PE_NONE;

			}

		} else if (player->pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += PXA_REVERSE;
			else if (dy < PXS_WALK) dy += PXA_WALK;
			else if (dy < PXS_RUN) dy += PXA_RUN;

		} else {

			// Sink

			dy += PYA_SINK;
			if (dy > PYS_SINK) dy = PYS_SINK;

		}

		if (dy < -PXS_RUN) dy = -PXS_RUN;
		else if (dy > PXS_RUN) dy = PXS_RUN;

	} else {

		if (platform && player->pcontrols[C_JUMP] &&
			!level->checkMaskUp(x + PXO_MID, y - F36)) {

			// Jump

			targetY = y - jumpHeight;

			// Increase jump height if walking/running
			if (dx < 0) targetY += dx >> 3;
			else if (dx > 0) targetY -= dx >> 3;

			event = JJ1PE_NONE;

			playSound(S_JUMPA);

		} else if (((event == JJ1PE_NONE) || (event == JJ1PE_PLATFORM)) &&
			!player->pcontrols[C_JUMP]) {

			// Stop jumping

			targetY = TTOF(LH);

		}

		if ((event == JJ1PE_FLOAT) &&
			player->pcontrols[C_DOWN]) {

			// Prevent floating

			dy = PYS_FALL;

		} else if (y >= targetY) {

			if ((event == JJ1PE_FLOAT) && (dy > PYS_JUMP)) {

					dy -= F16 + F8;

			} else {

				dy = (targetY - y - F64) * 4;

			}

			// Spring/float up speed limit
			if ((event == JJ1PE_SPRING) || (event == JJ1PE_REPELUP)) {

				speed = level->getEvent(eventX, eventY)->multiA * -F20;

				if (speed >= 0) speed = PYS_JUMP;

				if (dy < speed) dy = speed;

			}

			// Avoid jumping too fast, unless caused by an event
			if ((event == JJ1PE_NONE) && (dy < PYS_JUMP)) dy = PYS_JUMP;

		} else if (event != JJ1PE_PLATFORM) {

			// Fall under gravity
			if (dy < 0) dy += PYA_GRAVITY;
			else dy = PYS_FALL;

		}

		// Don't descend through platforms
		if ((dy > 0) && (event == JJ1PE_PLATFORM)) dy = 0;

		if (platform && !lookTime) {

			// If requested, look up or down
			if (player->pcontrols[C_UP]) lookTime = -ticks;
			else if (player->pcontrols[C_DOWN]) lookTime = ticks;

		}

		// Stop looking if there is no platform or the control has been released
		if (!platform ||
			(!player->pcontrols[C_UP] && (lookTime < 0)) ||
			(!player->pcontrols[C_DOWN] && (lookTime > 0))) lookTime = 0;

	}

	// If there is an obstacle above, stop rising
	if ((targetY < y) && checkMaskUp(PYO_TOP - F4)) {

		targetY = TTOF(LH);
		if (dy < 0) dy = 0;

		if (event != JJ1PE_PLATFORM) event = JJ1PE_NONE;

	}


	// Handle firing
	if (player->pcontrols[C_FIRE]) {

		if (ticks > fireTime) {

			if (player->ammoType == 4) {

				JJ1Event* event;

				// TNT

				event = level->getEvents();

				while (event) {

					// If the event is within range, hit it
					if (event->overlap(x - F160, y - F100, 2 * F160, 2 * F100)) {

						event->hit(this, 2, ticks);

					}

					event = event->getNext();

				}

				// Red flash
				level->flash(255, 0, 0, T_TNT);

			} else {

				// Horizontal bullet position is taken from the shooting animation
				animType = facing? PA_RSHOOT: PA_LSHOOT;

				level->createBullet(this,
					0,
					0,
					x + anims[animType]->getShootX(),
					y - ITOF(lookTime? 5: 10),
					player->getAmmo(false) + 1,
					facing,
					ticks);

			}

			// Set when the next bullet can be fired
			if (player->fireSpeed) fireTime = ticks + (1000 / player->fireSpeed);
			else fireTime = 0x7FFFFFFF;

			// Remove the bullet from the arsenal
			if (player->ammoType != -1) player->ammo[player->ammoType]--;

			/* If the current ammo type has been exhausted or TNT has been used,
			use the previous non-exhausted ammo type */
			while (((player->ammoType > -1) && !player->ammo[player->ammoType]) || (player->ammoType == 4)) player->ammoType--;

		}

	} else fireTime = 0;


	// Check for a change in ammo
	if (player->pcontrols[C_CHANGE]) {

		if (player == localPlayer) controls.release(C_CHANGE);

		player->ammoType = ((player->ammoType + 2) % 6) - 1;

		// If there is no ammo of this type, go to the next type that has ammo
		while ((player->ammoType > -1) && !player->ammo[player->ammoType])
			player->ammoType = ((player->ammoType + 2) % 6) - 1;

	}


	// Deal with the bird

	if (birds) birds = birds->step(ticks);


	return;

}


/**
 * Move the player.
 *
 * @param ticks Time
 */
void JJ1LevelPlayer::move (unsigned int ticks) {

	fixed pdx, pdy;
	bool grounded = false;
	int count;

	if (warpTime && (ticks > warpTime)) {

		x = TTOF(warpX);
		y = TTOF(warpY + 1);
		warpTime = 0;

	}

	// Apply as much of the trajectory as possible, without going into the
	// scenery

	if (fastFeetTime > ticks) {

		pdx = (udx * 3) >> 7;
		pdy = (dy * 3) >> 7;

	} else {

		pdx = udx >> 6;
		pdy = dy >> 6;

	}

	// First for the vertical component of the trajectory

	if (pdy < 0) {

		// Moving up

		count = (-pdy) >> 12;

		while (count > 0) {

			if (checkMaskUp(PYO_TOP - F4)) {

				y &= ~4095;
				dy = 0;

				break;

			}

			y -= F4;
			count--;

		}

		pdy = (-pdy) & 4095;

		if (checkMaskUp(PYO_TOP - pdy)) {

			y &= ~4095;
			dy = 0;

		} else y -= pdy;

	} else {

		if (pdy > 0) {

			// Moving down

			count = pdy >> 12;

			while (count > 0) {

				if (checkMaskDown(F4)) {

					y |= 4095;
					dy = 0;

					break;

				}

				y += F4;
				count--;

			}

			pdy &= 4095;

			if (checkMaskDown(pdy)) {

				y |= 4095;
				dy = 0;

			} else y += pdy;

		}

		if (!flying) {

			if (checkMaskDown(0)) {

				// In the ground, so move up
				if (y >= 4096) y = (y - 4096) | 4095;
				grounded = true;

			} else if (checkMaskDown(1)) {

				// On the ground
				grounded = true;

			}

		}

	}



	// Then for the horizontal component of the trajectory

	dx = udx;

	if (pdx < 0) {

		// Moving left

		count = (-pdx) >> 12;

		while (count > 0) {

			// If there is an obstacle, stop
			if (level->checkMaskUp(x + PXO_L - F4, y + PYO_MID)) {

				x &= ~4095;
				dx = 0;

				if (udx < -PXS_RUN) udx = -PXS_RUN;

				break;

			}

			x -= F4;
			count--;

			if (grounded) ground();

		}

		pdx = (-pdx) & 4095;

		if (level->checkMaskUp(x + PXO_L - pdx, y + PYO_MID)) {

			x &= ~4095;
			dx = 0;

			if (udx < -PXS_RUN) udx = -PXS_RUN;

		} else x -= pdx;

		if (grounded) ground();

	} else if (pdx > 0) {

		// Moving right

		count = pdx >> 12;

		while (count > 0) {

			// If there is an obstacle, stop
			if (level->checkMaskUp(x + PXO_R + F4, y + PYO_MID)) {

				x |= 4095;
				dx = 0;

				if (udx > PXS_RUN) udx = PXS_RUN;

				break;

			}

			x += F4;
			count--;

			if (grounded) ground();

		}

		pdx &= 4095;

		if (level->checkMaskUp(x + PXO_R + pdx, y + PYO_MID)) {

			x |= 4095;
			dx = 0;

			if (udx > PXS_RUN) udx = PXS_RUN;

		} else x += pdx;

		if (grounded) ground();

	}


	// If the target has been reached, stop rising
	if (y <= targetY) {

		targetY = TTOF(LH);

		if ((event != JJ1PE_PLATFORM) &&
			(event != JJ1PE_FLOATH) &&
			(event != JJ1PE_REPELH)) event = JJ1PE_NONE;

	}


	if (level->getStage() != LS_END) {

		// If the player has hit the bottom of the level, kill
		if (y + F4 > TTOF(LH)) kill(NULL, ticks);

		// Handle spikes
		if (level->checkSpikes(x + PXO_MID, y + PYO_TOP - F4) ||
			level->checkSpikes(x + PXO_MID, y + F4) ||
			level->checkSpikes(x + PXO_L - F4, y + PYO_MID) ||
			level->checkSpikes(x + PXO_R + F4, y + PYO_MID)) hit(NULL, ticks);

	}


	// Choose animation

	if (!energy)
		animType = facing? PA_RDIE: PA_LDIE;

	else if ((reaction == PR_HURT) && (reactionTime - ticks > PRT_HURT - PRT_HURTANIM))
		animType = facing? PA_RHURT: PA_LHURT;

	else if (y + PYO_MID > level->getWaterLevel() + F8)
		animType = facing? PA_RSWIM: PA_LSWIM;

	else if (flying) animType = facing? PA_RBOARD: PA_LBOARD;

	else if (dy < 0) {

		if (event == JJ1PE_SPRING) animType = facing? PA_RSPRING: PA_LSPRING;
		else animType = facing? PA_RJUMP: PA_LJUMP;

	} else if (checkMaskDown(F4) || (event == JJ1PE_PLATFORM)) {

		if (udx) {

			if (udx <= -PXS_RUN) animType = PA_LRUN;
			else if (udx >= PXS_RUN) animType = PA_RRUN;
			else if ((udx < 0) && facing) animType = PA_LSTOP;
			else if ((udx > 0) && !facing) animType = PA_RSTOP;
			else animType = facing? PA_RWALK: PA_LWALK;

		} else if (!level->checkMaskDown(x + PXO_ML, y + F20) &&
			!level->checkMaskDown(x + PXO_L, y + F2) &&
			(event != JJ1PE_PLATFORM))
			animType = PA_LEDGE;

		else if (!level->checkMaskDown(x + PXO_MR, y + F20) &&
			!level->checkMaskDown(x + PXO_R, y + F2) &&
			(event != JJ1PE_PLATFORM))
			animType = PA_REDGE;

		else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime))
			animType = PA_LOOKUP;

		else if (lookTime > 0) {

			if ((int)ticks < 1000 + lookTime) animType = facing? PA_RCROUCH: PA_LCROUCH;
			else animType = PA_LOOKDOWN;

		}

		else if (player->pcontrols[C_FIRE])
			animType = facing? PA_RSHOOT: PA_LSHOOT;

		else
			animType = facing? PA_RSTAND: PA_LSTAND;

	} else animType = facing? PA_RFALL: PA_LFALL;


	return;

}


/**
 * Calculate viewport.
 *
 * @param ticks Time
 * @param mspf Ticks per frame
 * @param change Time since last step
 */
void JJ1LevelPlayer::view (unsigned int ticks, int mspf, int change) {

	int oldViewX, oldViewY, speed;

	// Record old viewport position for applying lag
	oldViewX = viewX;
	oldViewY = viewY;

	// Find new position

	viewX = x + ((dx * change) >> 10) + F8 - (canvasW << 9);
	viewY = y + ((dy * change) >> 10) - F24 - ((canvasH - 33) << 9);

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

	if (speed && (mspf < speed)) {

		viewX = ((oldViewX * (speed - mspf)) + (viewX * mspf)) / speed;
		viewY = ((oldViewY * (speed - mspf)) + (viewY * mspf)) / speed;

	}


	return;

}

/**
 * Draw the player.
 *
 * @param ticks Time
 * @param change Time since last step
 */
void JJ1LevelPlayer::draw (unsigned int ticks, int change) {

	Anim *an;
	int frame;
	fixed drawX, drawY;
	fixed xOffset, yOffset;

	// The current frame for animations
	if (reaction == PR_KILLED) frame = (ticks + PRT_KILLED - reactionTime) / 75;
	else frame = ticks / 75;


	// Get position

	drawX = getDrawX(change);
	drawY = getDrawY(change);


	// Choose sprite

	an = anims[animType];
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
	/*drawRect(FTOI(drawX + PXO_L),
		FTOI(drawY + PYO_TOP),
		FTOI(PXO_R - PXO_L),
		FTOI(-PYO_TOP), 89);
	drawRect(FTOI(drawX + PXO_ML),
		FTOI(drawY + PYO_TOP),
		FTOI(PXO_MR - PXO_ML),
		FTOI(-PYO_TOP), 88);*/

	// Uncomment the following to show the tile containing the player's base
	//drawRect(FTOI(TTOF(FTOT(x + PXO_MID)) - viewX), FTOI(TTOF(FTOT(y)) - viewY), 32, 32, 48);

	// Uncomment the following to show the player's event tile
	// if (event != JJ1PE_NONE) drawRect(FTOI(TTOF(eventX) - viewX), FTOI(TTOF(eventY) - viewY), 32, 32, 89);


	if (flying) {

		an = level->getMiscAnim(facing? MA_RBOARD: MA_LBOARD);

		an->setFrame(ticks >> 4, true);
		an->draw(drawX, drawY + F10);

	}

	if (reaction == PR_INVINCIBLE) {

		// Show invincibility stars

		xOffset = fSin(ticks * 2) * 12;
		yOffset = fCos(ticks * 2) * 12;

		an = level->getMiscAnim(MA_SPARKLE);

		an->setFrame(frame, true);
		an->draw(drawX + PXO_MID + xOffset, drawY + PYO_MID + yOffset);

		an->setFrame(frame + 1, true);
		an->draw(drawX + PXO_MID - xOffset, drawY + PYO_MID - yOffset);

		an->setFrame(frame + 2, true);
		an->draw(drawX + PXO_MID + yOffset, drawY + PYO_MID + xOffset);

		an->setFrame(frame + 3, true);
		an->draw(drawX + PXO_MID - yOffset, drawY + PYO_MID - xOffset);

	} else if (shield > 1) {

		// Show the 4-hit shield

		xOffset = fCos(ticks) * 20;
		yOffset = fSin(ticks) * 20;

		an = level->getAnim(59);

		an->draw(drawX + xOffset, drawY + PYO_TOP + yOffset);

		if (shield > 2) an->draw(drawX - xOffset, drawY + PYO_TOP - yOffset);

		if (shield > 3) an->draw(drawX + yOffset, drawY + PYO_TOP - xOffset);

		if (shield > 4) an->draw(drawX - yOffset, drawY + PYO_TOP + xOffset);

	} else if (shield) {

		// Show the 1-hit shield

		xOffset = fCos(ticks) * 20;
		yOffset = fSin(ticks) * 20;

		an = level->getAnim(50);

		an->draw(drawX + xOffset, drawY + PYO_TOP + yOffset);
		an->draw(drawX - xOffset, drawY + PYO_TOP - yOffset);

	}


	// Show the bird
	if (birds) birds->draw(ticks, change);


	// Show the player's name
	if (nPlayers > 1)
		panelBigFont->showString(player->name,
			FTOI(drawX + PXO_MID) - (panelBigFont->getStringWidth(player->name) >> 1),
			FTOI(drawY - F32 - F16));

	return;

}


