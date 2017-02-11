
/**
 *
 * @file jj2levelplayerframe.cpp
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


#include "../jj2event/jj2event.h"
#include "../jj2level.h"
#include "jj2levelplayer.h"

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
bool JJ2LevelPlayer::checkMaskDown (fixed yOffset, bool drop) {

	return jj2Level->checkMaskDown(x + JJ2PXO_ML, y + yOffset, drop) ||
		jj2Level->checkMaskDown(x + JJ2PXO_MID, y + yOffset, drop) ||
		jj2Level->checkMaskDown(x + JJ2PXO_MR, y + yOffset, drop);

}


/**
 * Determine whether or not the area above the player is solid when travelling
 * upwards.
 *
 * @param yOffset Vertical offset of the mask values to check
 *
 * @return Solidity
 */
bool JJ2LevelPlayer::checkMaskUp (fixed yOffset) {

	return jj2Level->checkMaskUp(x + JJ2PXO_ML, y + yOffset) ||
		jj2Level->checkMaskUp(x + JJ2PXO_MID, y + yOffset) ||
		jj2Level->checkMaskUp(x + JJ2PXO_MR, y + yOffset);

}


/**
 * Move the player to the ground's surface.
 */
void JJ2LevelPlayer::ground () {

	// If on an uphill slope, push the player upwards
	if (checkMaskUp(0) && !checkMaskUp(-F1)) y -= F1;

	// If on a downhill slope, push the player downwards
	if (!checkMaskUp(F1) && checkMaskUp(F2)) y += F1;

	return;

}


/**
 * Respond to tile modifier events.
 *
 * @param nextMod The modifier event
 * @param ticks Time
 */
void JJ2LevelPlayer::modify (JJ2Modifier* nextMod, unsigned int ticks) {

	switch (nextMod->type) {

		case 2: // Spikes

			if (jj2Level->checkMaskDown(x + JJ2PXO_MID, y + F1, false))
				hit(NULL, ticks);

			break;

		case 4: // Hook

			dx = 0;

			break;

		case 6: // H-pole

			if (!stopTime) {

				// Catch player
				if (nextMod != mod) {

					centreX();
					centreY();
					stopTime = ticks + 1000;

				}

			} else if (ticks > stopTime) {

				dx = (dx > 0) ? JJ2PXS_POLE: -JJ2PXS_POLE;
				stopTime = 0;

			}

			break;

		case 7: // V-pole

			if (!stopTime) {

				// Catch player
				if (nextMod != mod) {

					centreX();
					centreY();
					stopTime = ticks + 1000;

				}

			} else if (ticks > stopTime) {

				if (dy < 0) {

					throwY = y - TTOF(16);
					dy = -JJ2PYS_POLE;

				} else {

					dy = JJ2PYS_POLE;

				}

				stopTime = 0;

			}

			break;

		case 8: // Fly off

			floating = false;

			break;

		case 17: // End of level
		case 18: // End of level

			if (!energy) return;

			if (!player->endOfLevel(FTOT(x + JJ2PXO_MID), FTOT(y + JJ2PYO_MID))) return;

			jj2Level->setStage(LS_END);

			break;

		case 206: // Sucker tube

			dx = (nextMod->properties & 0x40) ? -((nextMod->properties << 15) & 0x1F8000): ((nextMod->properties << 15) & 0x1F8000);
			dy = (nextMod->properties & 0x2000) ? -((nextMod->properties << 8) & 0x1F8000): ((nextMod->properties << 8) & 0x1F8000);

			if (dx) centreY();
			if (dy) centreX();

			break;

		case 207: // Text

			break;

		case 208: // Water level

			jj2Level->setWaterLevel(nextMod->properties & 0xFF, nextMod->properties & 0x100);

			break;

		case 230: // Warp

			if (!stopTime) {

				// Catch player
				if (coins >= ((nextMod->properties >> 8) & 255)) stopTime = ticks + 1000;

			} else if (ticks > stopTime) {

				coins -= (nextMod->properties >> 8) & 255;
				jj2Level->warp(this, nextMod->properties & 255);

				stopTime = 0;

			}

			break;

		default:

			stopTime = 0;

			break;

	}

	mod = nextMod;

	return;

}


/**
 * Respond to controls, unless the player has been killed
 *
 * @param ticks Time
 * @param msps Ticks per step
 */
void JJ2LevelPlayer::control (unsigned int ticks, int msps) {

	JJ2Modifier* nextMod;
	bool drop, platform;


	// If the player has been killed, do not move
	if (!energy) {

		dx = 0;
		dy = 0;

		animType = JJ2PA_DIE;

		return;

	}


	// Get overlapping modifier
	nextMod = jj2Level->getModifier(FTOT(x + JJ2PXO_MID), FTOT(y + JJ2PYO_MID));


	if (stopTime) {

		// Can't control player, so just apply modifier

		modify(nextMod, ticks);

		return;

	}


	if (player->pcontrols[C_RIGHT]) {

		// Walk/run right

		if (dx < 0) dx += JJ2PXA_REVERSE * msps;
		else if (dx < JJ2PXS_WALK) dx += JJ2PXA_WALK * msps;
		else if (dx < JJ2PXS_RUN) dx += JJ2PXA_RUN * msps;

		facing = true;

	} else if (player->pcontrols[C_LEFT]) {

		// Walk/run left

		if (dx > 0) dx -= JJ2PXA_REVERSE * msps;
		else if (dx > -JJ2PXS_WALK) dx -= JJ2PXA_WALK * msps;
		else if (dx > -JJ2PXS_RUN) dx -= JJ2PXA_RUN * msps;

		facing = false;

	} else if ((nextMod->type >> 1) != 3) {

		// Slow down

		if (dx > 0) {

			if (dx < JJ2PXA_STOP * msps) dx = 0;
			else dx -= JJ2PXA_STOP * msps;

		}

		if (dx < 0) {

			if (dx > -JJ2PXA_STOP * msps) dx = 0;
			else dx += JJ2PXA_STOP * msps;

		}

	}


	drop = player->pcontrols[C_DOWN];

	// Check for platform event, bridge or level mask below player
	platform = (event == JJ2PE_PLATFORM) ||
		checkMaskDown(1, drop) ||
		((dx > 0) && jj2Level->checkMaskDown(x + JJ2PXO_ML, y + F8, drop)) ||
		((dx < 0) && jj2Level->checkMaskDown(x + JJ2PXO_MR, y + F8, drop));

	if (floating) {

		if (player->pcontrols[C_UP]) {

			// Fly upwards

			if (dy > 0) dy -= JJ2PXA_REVERSE * msps;
			else if (dy > -JJ2PXS_WALK) dy -= JJ2PXA_WALK * msps;
			else if (dy > -JJ2PXS_RUN) dy -= JJ2PXA_RUN * msps;

		} else if (player->pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += JJ2PXA_REVERSE * msps;
			else if (dy < JJ2PXS_WALK) dy += JJ2PXA_WALK * msps;
			else if (dy < JJ2PXS_RUN) dy += JJ2PXA_RUN * msps;

		} else if ((nextMod->type >> 1) != 3) {

			// Slow down

			if (dy > 0) {

				if (dy < JJ2PXA_STOP * msps) dy = 0;
				else dy -= JJ2PXA_STOP * msps;

			}

			if (dy < 0) {

				if (dy > -JJ2PXA_STOP * msps) dy = 0;
				else dy += JJ2PXA_STOP * msps;

			}

		}

		if (event == JJ2PE_SPRING) dy = JJ2PYS_SPRING;
		else if (event == JJ2PE_FLOAT) dy = JJ2PYS_JUMP;

	} else if (y + JJ2PYO_MID > jj2Level->getWaterLevel()) {

		if (player->pcontrols[C_SWIM]) {

			// Swim upwards

			if (dy > 0) dy -= JJ2PXA_REVERSE * msps;
			else if (dy > -JJ2PXS_WALK) dy -= JJ2PXA_WALK * msps;
			else if (dy > -JJ2PXS_RUN) dy -= JJ2PXA_RUN * msps;

			// Prepare to jump upon leaving the water

			if (!jj2Level->checkMaskUp(x + JJ2PXO_MID, y - F36)) {

				throwY = y - jumpHeight;

				if (dx < 0) throwY += dx >> 4;
				else if (dx > 0) throwY -= dx >> 4;

				event = JJ2PE_NONE;

			}

		} else if (player->pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += JJ2PXA_REVERSE * msps;
			else if (dy < JJ2PXS_WALK) dy += JJ2PXA_WALK * msps;
			else if (dy < JJ2PXS_RUN) dy += JJ2PXA_RUN * msps;

		} else {

			// Sink

			dy += JJ2PYA_SINK * msps;
			if (dy > JJ2PYS_SINK) dy = JJ2PYS_SINK;

		}

	} else {

		if (platform && player->pcontrols[C_JUMP] &&
			!jj2Level->checkMaskUp(x + JJ2PXO_MID, y - F36)) {

			// Jump

			throwY = y - jumpHeight;

			// Increase jump height if walking/running
			if (dx < 0) throwY += dx >> 3;
			else if (dx > 0) throwY -= dx >> 3;

			event = JJ2PE_NONE;

			playSound(S_JUMPA);

		}

		// Stop jumping
		if (!player->pcontrols[C_JUMP] && (event != JJ2PE_SPRING) && (event != JJ2PE_FLOAT))
			throwY = TTOF(256);

		if (y >= throwY) {

			// If jumping, rise
			dy = (throwY - y - F64) * 4;

			// Avoid jumping too fast, unless caused by an event
			if ((event == JJ2PE_NONE) && (dy < JJ2PYS_JUMP)) dy = JJ2PYS_JUMP;

		} else if (!platform) {

			// Fall under gravity
			dy += JJ2PYA_GRAVITY * msps;
			if (dy > JJ2PYS_FALL) dy = JJ2PYS_FALL;

		}

		// Don't descend through platforms
		if ((dy > 0) && (event == JJ2PE_PLATFORM)) dy = 0;

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

	// If there is an obstacle above and the player is not floating up, stop
	// rising
	if (jj2Level->checkMaskUp(x + JJ2PXO_MID, y + JJ2PYO_TOP - F1) && (throwY < y) && (event != JJ2PE_FLOAT)) {

		throwY = TTOF(256);
		if (dy < 0) dy = 0;

		if (event != JJ2PE_PLATFORM) event = JJ2PE_NONE;

	}

	// If jump completed, stop rising
	if (y <= throwY) {

		throwY = TTOF(256);

		if (event != JJ2PE_PLATFORM) event = JJ2PE_NONE;

	}


	// Apply modifier
	modify(nextMod, ticks);


	// Limit speed

	if (dx < -JJ2PXS_LIMIT) dx = -JJ2PXS_LIMIT;
	else if (dx > JJ2PXS_LIMIT) dx = JJ2PXS_LIMIT;

	if (dy < -JJ2PYS_LIMIT) dy = -JJ2PYS_LIMIT;
	else if (dy > JJ2PYS_LIMIT) dy = JJ2PYS_LIMIT;


	// Handle firing
	if (player->pcontrols[C_FIRE]) {

		if (ticks > fireTime) {

			// Make sure bullet position is taken from correct animation
			if (platform) animType = JJ2PA_STANDSHOOT;

			/// @todo Create new bullet when firing

			// Set when the next bullet can be fired
			if (player->fireSpeed) fireTime = ticks + (1000 / player->fireSpeed);
			else fireTime = 0x7FFFFFFF;

			// Remove the bullet from the arsenal
			if (player->ammoType != -1) player->ammo[player->ammoType]--;

			/* If the current ammo type has been exhausted, use the previous
			non-exhausted ammo type */
			while ((player->ammoType > -1) && !player->ammo[player->ammoType]) player->ammoType--;

		}

	} else fireTime = 0;


	// Check for a change in ammo
	if (player->pcontrols[C_CHANGE]) {

		if (player == localPlayer) controls.release(C_CHANGE);

		player->ammoType = ((player->ammoType + 2) % 5) - 1;

		// If there is no ammo of this type, go to the next type that has ammo
		while ((player->ammoType > -1) && !player->ammo[player->ammoType])
			player->ammoType = ((player->ammoType + 2) % 5) - 1;

	}


	// Deal with the bird

	//if (birds) birds = birds->step(ticks, msps);


	// Choose animation

	if ((reaction == JJ2PR_HURT) && (reactionTime - ticks > JJ2PRT_HURT - JJ2PRT_HURTANIM))
		animType = JJ2PA_HURT;

	else if (y + JJ2PYO_MID > jj2Level->getWaterLevel())
		animType = JJ2PA_SWIM;

	else if (floating) animType = JJ2PA_BOARD;

	else if (dy < 0) {

		if (event == JJ2PE_SPRING) animType = JJ2PA_FLOAT1;
		else animType = JJ2PA_JUMP;

	} else if (platform) {

		if (dx) {

			if (dx <= -JJ2PXS_RUN) animType = JJ2PA_RUN;
			else if (dx >= JJ2PXS_RUN) animType = JJ2PA_RUN;
			else if ((dx < 0) && facing) animType = JJ2PA_STOP1;
			else if ((dx > 0) && !facing) animType = JJ2PA_STOP1;
			else animType = JJ2PA_WALKSHOOT;

		} else if (!jj2Level->checkMaskDown(x + JJ2PXO_ML, y + F12, drop) &&
			!jj2Level->checkMaskDown(x + JJ2PXO_L, y + F2, drop) &&
			(event != JJ2PE_PLATFORM))
			animType = JJ2PA_EDGE;

		else if (!jj2Level->checkMaskDown(x + JJ2PXO_MR, y + F12, drop) &&
			!jj2Level->checkMaskDown(x + JJ2PXO_R, y + F2, drop) &&
			(event != JJ2PE_PLATFORM))
			animType = JJ2PA_EDGE;

		else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime))
			animType = JJ2PA_LOOKUP;

		else if (lookTime > 0) {

			if ((int)ticks < 1000 + lookTime) animType = JJ2PA_CROUCHED;
			else animType = JJ2PA_CROUCH1;

		}

		else if (player->pcontrols[C_FIRE]) animType = JJ2PA_STANDSHOOT;

		else animType = JJ2PA_STAND;

	} else animType = JJ2PA_FALL;


	return;

}


/**
 * Move the player.
 *
 * @param ticks Time
 * @param msps Ticks per step
 */
void JJ2LevelPlayer::move (unsigned int ticks, int msps) {

	fixed pdx, pdy;
	bool grounded = false;
	int count;
	bool drop;


	if (stopTime) return;


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

	drop = player->pcontrols[C_DOWN];

	if (pdy < 0) {

		// Moving up

		count = (-pdy) >> 10;

		while (count > 0) {

			if (jj2Level->checkMaskUp(x + JJ2PXO_MID, y + JJ2PYO_TOP - F1)) {

				y &= ~1023;
				dy = 0;

				break;

			}

			y -= F1;
			count--;

		}

		pdy = (-pdy) & 1023;

		if (jj2Level->checkMaskUp(x + JJ2PXO_MID, y + JJ2PYO_TOP - pdy)) {

			y &= ~1023;
			dy = 0;

		} else y -= pdy;

	} else {

		if (pdy > 0) {

			// Moving down

			count = pdy >> 10;

			while (count > 0) {

				if (checkMaskDown(F1, drop)) {

					y |= 1023;
					dy = 0;

					break;

				}

				y += F1;
				count--;

			}

			pdy &= 1023;

			if (checkMaskDown(pdy, drop)) y |= 1023;
			else y += pdy;

		}

		if (checkMaskDown(0, drop)) {

			// In the ground, so move up
			if (y >= 1024) y = (y - 1024) | 1023;

			dy = 0;
			grounded = true;

		} else if (checkMaskDown(1, drop)) {

			// On the ground
			dy = 0;
			grounded = true;

		}

	}



	// Then for the horizontal component of the trajectory

	if (pdx < 0) {

		// Moving left

		count = (-pdx) >> 10;

		while (count > 0) {

			// If there is an obstacle, stop
			if (jj2Level->checkMaskUp(x + JJ2PXO_L - F1, y + JJ2PYO_MID)) {

				x &= ~1023;
				dx = 0;

				break;

			}

			x -= F1;
			count--;

			if (grounded) ground();

		}

		pdx = (-pdx) & 1023;

		if (jj2Level->checkMaskUp(x + JJ2PXO_L - pdx, y + JJ2PYO_MID)) {

			x &= ~1023;
			dx = 0;

		} else x -= pdx;

		if (grounded) ground();

	} else if (pdx > 0) {

		// Moving right

		count = pdx >> 10;

		while (count > 0) {

			// If there is an obstacle, stop
			if (jj2Level->checkMaskUp(x + JJ2PXO_R + F1, y + JJ2PYO_MID)) {

				x |= 1023;
				dx = 0;

				break;

			}

			x += F1;
			count--;

			if (grounded) ground();

		}

		pdx &= 1023;

		if (jj2Level->checkMaskUp(x + JJ2PXO_R + pdx, y + JJ2PYO_MID)) {

			x |= 1023;
			dx = 0;

		} else x += pdx;

		if (grounded) ground();

	}


	// If using a float up event and have hit a ceiling, ignore event
	if ((event == JJ2PE_FLOAT) && jj2Level->checkMaskUp(x + JJ2PXO_MID, y + JJ2PYO_TOP - F1)) {

		throwY = TTOF(256);
		event = JJ2PE_NONE;

	}


	if (jj2Level->getStage() == LS_END) return;


	return;

}


/**
 * Calculate viewport.
 *
 * @param ticks Time
 * @param mspf Ticks per frame
 * @param change Time since last step
 */
void JJ2LevelPlayer::view (unsigned int ticks, int mspf, int change) {

	int oldViewX, oldViewY, speed;

	// Record old viewport position for applying lag
	oldViewX = viewX;
	oldViewY = viewY;


	// Find new position

	viewX = x + ((dx * change) >> 10) + F8 - (canvasW << 9);
	viewY = y + ((dy * change) >> 10) - F24 - (canvasH << 9);

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
void JJ2LevelPlayer::draw (unsigned int ticks, int change) {

	Anim *an;
	int frame;
	fixed drawX, drawY;

	// The current frame for animations
	if (reaction == JJ2PR_KILLED) frame = (ticks + JJ2PRT_KILLED - reactionTime) / 75;
	else frame = ticks / 75;


	// Get position

	drawX = getDrawX(stopTime? 0: change);
	drawY = getDrawY(stopTime? 0: change);


	// Choose sprite

	an = getAnim();
	an->setFrame(frame, reaction != JJ2PR_KILLED);


	// Show the player

	// Use player colour
	an->setPalette(palette, 16, 32);

	// Flash on and off if hurt
	if ((reaction != JJ2PR_HURT) || ((ticks / 30) & 2)) {

		// Draw "motion blur"
		if (fastFeetTime > ticks) an->draw(drawX + F16 - (dx >> 6), drawY + F16);

		// Draw player
		an->draw(drawX + F16, drawY + F16);

	}


	// Uncomment the following to see the area of the player
	/*drawRect(FTOI(drawX + JJ2PXO_L - viewX),
		FTOI(drawY + JJ2PYO_TOP - viewY),
		FTOI(JJ2PXO_R - JJ2PXO_L),
		FTOI(-JJ2PYO_TOP), 89);
	drawRect(FTOI(drawX + JJ2PXO_ML - viewX),
		FTOI(drawY + JJ2PYO_TOP - viewY),
		FTOI(JJ2PXO_MR - JJ2PXO_ML),
		FTOI(-JJ2PYO_TOP), 88);*/


	if (reaction == JJ2PR_INVINCIBLE) {

		/// @todo Show invincibility effect

	}

	switch (shield) {

		case JJ2S_NONE:

			// Do nothing

			break;

		case JJ2S_FLAME:

			/// @todo Show shield effect

			break;

		case JJ2S_BUBBLE:

			/// @todo Show shield effect

			break;

		case JJ2S_PLASMA:

			/// @todo Show shield effect

			break;

		case JJ2S_LASER:

			/// @todo Show shield effect

			break;

	}


	// Show the bird
	//if (birds) birds->draw(ticks, change);


	// Show the player's name
	if (nPlayers > 1)
		panelBigFont->showString(player->name,
			FTOI(drawX + JJ2PXO_MID) - (panelBigFont->getStringWidth(player->name) >> 1),
			FTOI(drawY - F32 - F16));

	//panelBigFont->showNumber(mod->properties, FTOI(drawX) + 24, FTOI(drawY) + 12);

	return;

}


