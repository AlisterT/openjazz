
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


#include "font.h"
#include "game.h"
#include "level.h"
#include "palette.h"
#include "sound.h"
#include <string.h>
#include <math.h>


Player::Player () {

	bird = NULL;

	name = NULL;

	return;

}


Player::~Player () {

	deinit();

	return;

}


void Player::init (char *playerName, unsigned char *playerCols,
	unsigned char newTeam) {

	int offsets[15] = {PC_WHITE, PC_SGREEN, PC_BLUE, PC_RED, PC_LGREEN,
		PC_LEVEL1, PC_YELLOW, PC_LEVEL2, PC_ORANGE, PC_LEVEL3, PC_LEVEL4,
		PC_SANIM, PC_LANIM, PC_LEVEL5, 256};
	int count, start, length;

	// Clear existing player
	deinit();

	// Assign name
	name = cloneString(playerName);

	// Assign initial values
	memset(anims, 0, PANIMS);
	score = 0;
	lives = 3;
	ammoType = -1;
	ammo[0] = 0;
	ammo[1] = 0;
	ammo[2] = 0;
	ammo[3] = 0;
	fireSpeed = 0;
	team = newTeam;
	teamScore = 0;


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;

	if (playerCols == NULL) return;

	memcpy(cols, playerCols, 4);

	// Fur colours

	count = 0;

	while (cols[0] >= offsets[count + 1]) count++;

	start = offsets[count];
	length = offsets[count + 1] - start;

	for (count = 0; count < 16; count++)
		palette[count + 48].r = palette[count + 48].g = palette[count + 48].b =
			(count * length / 16) + start;


	// Bandana colours

	count = 0;

	while (cols[1] >= offsets[count + 1]) count++;

	start = offsets[count];
	length = offsets[count + 1] - start;

	for (count = 0; count < 16; count++)
		palette[count + 32].r = palette[count + 32].g = palette[count + 32].b =
 			(count * length / 16) + start;


	// Gun colours

	count = 0;

	while (cols[2] >= offsets[count + 1]) count++;

	start = offsets[count];
	length = offsets[count + 1] - start;

	for (count = 0; count < 9; count++)
		palette[count + 23].r = palette[count + 23].g = palette[count + 23].b =
			(count * length / 9) + start;


	// Wristband colours

	count = 0;

	while (cols[3] >= offsets[count + 1]) count++;

	start = offsets[count];
	length = offsets[count + 1] - start;

	for (count = 0; count < 8; count++)
		palette[count + 88].r = palette[count + 88].g = palette[count + 88].b =
			(count * length / 8) + start;


	return;

}


void Player::deinit () {

	if (bird) delete bird;
	bird = NULL;

	if (name) delete[] name;
	name = NULL;

	return;

}


void Player::setAnims (char *newAnims) {

	memcpy(anims, newAnims, PANIMS);

	return;

}


char * Player::getName () {

	return name;

}


unsigned char * Player::getCols () {

	return cols;

}


void Player::reset () {

	int count;

	if (bird) bird->reset();

	event = NULL;

	for (count = 0; count < PCONTROLS; count++) pcontrols[count] = false;

	energy = 4;
	shield = 0;
	floating = false;
	facing = true;
	reaction = PR_NONE;
	reactionTime = 0;
	jumpHeight = 92 * F1;
	jumpY = LH * F32;
	fastFeetTime = 0;
	warpTime = 0;
	dx = 0;
	dy = 0;
	enemies = items = 0;

	return;

}


void Player::setControl (int control, bool state) {

	pcontrols[control] = state;

	return;

}


void Player::shootEvent (unsigned char gridX, unsigned char gridY, int ticks) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	addScore(set[E_ADDEDSCORE]);

	switch (set[E_MODIFIER]) {

		case 41: // Bonus level

			if (getEnergy()) level->setNext(set[E_MULTIPURPOSE], set[E_YAXIS]);

			// The lack of a break statement is intentional

		case 8: // Boss
		case 27: // End of level

			if (getEnergy()) {

				setCheckpoint(gridX, gridY);
				reaction = PR_WON;
				reactionTime = ticks + PRT_WON;

				level->win();

			}

			break;

		case 10: // Checkpoint

			setCheckpoint(gridX, gridY);

			break;

		case 15:

			addAmmo(0, 15);

			break;

		case 16:

			addAmmo(1, 15);

			break;

		case 17:

			addAmmo(2, 15);

			break;

		case 26:

			fastFeetTime = ticks + T_FASTFEET;

			break;

		case 33:

			if (shield < 2) shield = 2;

			break;

		case 34: // Bird

			if (!bird) bird = new Bird(this, gridX, gridY);

			break;

		case 36:

			shield = 6;

			break;

	}

	// Add to player's enemy/item tally
	// If the event hurts and can be killed, it is an enemy
	// Anything else that scores is an item
	if ((set[E_MODIFIER] == 0) && set[E_HITSTOKILL]) enemies++;
	else if (set[E_ADDEDSCORE]) items++;

	return;

}


bool Player::touchEvent (unsigned char gridX, unsigned char gridY, int ticks) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	switch (set[E_MODIFIER]) {

		case 0: // Hurt
		case 8: // Boss

			if ((set[E_BEHAVIOUR] < 37) || (set[E_BEHAVIOUR] > 44)) hit(ticks);

			break;

		case 1: // Invincibility

			if (getEnergy()) {

				reaction = PR_INVINCIBLE;
				reactionTime = ticks + PRT_INVINCIBLE;
				addScore(set[E_ADDEDSCORE]);

				return true;

			}

			break;

		case 2:
		case 3: // Health

			if (energy < 4) energy++;
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 4: // Extra life

			if (lives < 99) lives++;
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 5: // High-jump feet

			jumpHeight += F16;

			return true;

		case 9: // Sand timer

			level->addTimer();
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 11: // Item

			addScore(set[E_ADDEDSCORE]);

			return true;

		case 12: // Rapid fire

			fireSpeed++;

			return true;

		case 13: // Warp

			if (!warpTime) {

				warpX = set[E_MULTIPURPOSE];
				warpY = set[E_YAXIS];
				warpTime = ticks + T_WARP;

				// White flash
				firstPE =
					new FlashPaletteEffect(255, 255, 255, T_WARP, firstPE);

			}

			break;

		case 18: // Ammo

			addAmmo(0, 2);
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 19: // Ammo

			addAmmo(1, 2);
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 20: // Ammo

			addAmmo(2, 2);
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 29: // Upwards spring

			setEvent(set);

			level->playSound(set[E_SOUND]);

			break;

		case 30: // TNT

			addAmmo(3, 1);

			return true;

		case 31: // Water level

			if (!set[E_HITSTOKILL]) level->setWaterLevel(gridY);

			break;

		case 35: // Airboard, etc.

			floating = true;
			addScore(set[E_ADDEDSCORE]);

			return true;

		case 37: // Diamond

			// Yellow flash
			firstPE = new FlashPaletteEffect(255, 255, 0, 320, firstPE);

			return true;

		case 38: // Airboard, etc. off

			floating = false;

			break;

	}

	return false;

}


bool Player::hit (int ticks) {

	if (reaction != PR_NONE) return false;

	if (shield == 3) shield = 0;
	else if (shield) shield--;
	else {

		energy--;

		if (bird) bird->hit();

		playSound(S_OW);

	}

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

	return true;

}


void Player::kill (int ticks) {

	if (reaction != PR_NONE) return;

	energy = 0;
	lives--;

	reaction = PR_KILLED;
	reactionTime = ticks + PRT_KILLED;

	if (!game || (game->getMode() == M_SINGLE))
		firstPE = new FadeOutPaletteEffect(T_END, firstPE);

	return;

}


void Player::addScore (int addedScore) {

	score += addedScore * 10;

	return;

}

int Player::getScore () {

	return score;

}


int Player::getEnergy () {

	return energy;

}


int Player::getLives () {

	return lives;

}


void Player::setCheckpoint (unsigned char gridX, unsigned char gridY) {

	unsigned char buffer[MTL_G_CHECK];

	checkX = gridX;
	checkY = gridY;

	if (game && (game->getMode() != M_SINGLE)) {

		buffer[0] = MTL_G_CHECK;
		buffer[1] = MT_G_CHECK;
		buffer[2] = gridX;
		buffer[3] = gridY;

		game->send(buffer);

	}

	return;

}


void Player::addAmmo (int type, int amount) {

	if (!ammo[type]) ammoType = type;
	ammo[type] += amount;

	return;

}


int Player::getAmmo (bool amount) {

	return amount? ammo[ammoType]: ammoType;

}


int Player::getEnemies () {

	return enemies;

}


int Player::getItems () {

	return items;

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
	if (newDy) dy = newDy;

	return;

}


bool Player::getFacing () {

	return facing;

}


unsigned char Player::getTeam () {

	return team;

}


void Player::floatUp (signed char *newEvent) {

	event = newEvent;

	if ((dy > 0) && level->checkMaskDown(x + PXO_MID, y + F4))
		dy = event[E_MULTIPURPOSE] * -F40;

	if (dy > event[E_MULTIPURPOSE] * -F40)
		dy -= event[E_MULTIPURPOSE] * 320 * mspf;

	jumpY = y - (8 * F16);

	return;

}


void Player::belt (int speed) {

	dx += speed * 160 * mspf;

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


void Player::send (unsigned char *data) {

	// Copy data to be sent to clients/server

	data[3] = pcontrols[C_UP];
	data[4] = pcontrols[C_DOWN];
	data[5] = pcontrols[C_LEFT];
	data[6] = pcontrols[C_RIGHT];
	data[7] = pcontrols[C_JUMP];
	data[8] = pcontrols[C_FIRE];
	data[9] = bird? 1: 0;
	data[10] = ammo[0] >> 8;
	data[11] = ammo[0] & 255;
	data[12] = ammo[1] >> 8;
	data[13] = ammo[1] & 255;
	data[14] = ammo[2] >> 8;
	data[15] = ammo[2] & 255;
	data[16] = ammo[3] >> 8;
	data[17] = ammo[3] & 255;
	data[18] = ammoType + 1;
	data[19] = score >> 24;
	data[20] = (score >> 16) & 255;
	data[21] = (score >> 8) & 255;
	data[22] = score & 255;
	data[23] = energy;
	data[24] = lives;
	data[25] = shield;
	data[26] = floating;
	data[27] = facing;
	data[28] = fireSpeed;
	data[29] = jumpHeight >> 24;
	data[30] = (jumpHeight >> 16) & 255;
	data[31] = (jumpHeight >> 8) & 255;
	data[32] = jumpHeight & 255;
	data[33] = jumpY >> 24;
	data[34] = (jumpY >> 16) & 255;
	data[35] = (jumpY >> 8) & 255;
	data[36] = jumpY & 255;
	data[37] = x >> 24;
	data[38] = (x >> 16) & 255;
	data[39] = (x >> 8) & 255;
	data[40] = x & 255;
	data[41] = y >> 24;
	data[42] = (y >> 16) & 255;
	data[43] = (y >> 8) & 255;
	data[44] = y & 255;

	return;

}


void Player::receive (unsigned char *buffer) {

	// Interpret data recieved from client/server

	switch (buffer[1]) {

		case MT_P_ANIMS:

			setAnims((char *)buffer + 3);

			break;

		case MT_P_TEMP:

			pcontrols[C_UP] = buffer[3];
			pcontrols[C_DOWN] = buffer[4];
			pcontrols[C_LEFT] = buffer[5];
			pcontrols[C_RIGHT] = buffer[6];
			pcontrols[C_JUMP] = buffer[7];
			pcontrols[C_FIRE] = buffer[8];
			pcontrols[C_CHANGE] = false;

			if ((buffer[9] & 1) && !bird)
				bird = new Bird(this, x >> 15, y >> 15);

			if (!(buffer[9] & 1) && bird) {

				delete bird;
				bird = NULL;

			}

			ammo[0] = (buffer[10] << 8) + buffer[11];
			ammo[1] = (buffer[12] << 8) + buffer[13];
			ammo[2] = (buffer[14] << 8) + buffer[15];
			ammo[3] = (buffer[16] << 8) + buffer[17];
			ammoType = buffer[18] - 1;
			score = (buffer[19] << 24) + (buffer[20] << 16) +
				(buffer[21] << 8) + buffer[22];
			energy = buffer[23];
			lives = buffer[24];
			shield = buffer[25];
			floating = buffer[26];
			facing = buffer[27];
			fireSpeed = buffer[28];
			jumpHeight = (buffer[29] << 24) + (buffer[30] << 16) +
				(buffer[31] << 8) + buffer[32];
			jumpY = (buffer[33] << 24) + (buffer[34] << 16) +
				(buffer[35] << 8) + buffer[36];
			x = (buffer[37] << 24) + (buffer[38] << 16) + (buffer[39] << 8) +
				buffer[40];
			y = (buffer[41] << 24) + (buffer[42] << 16) + (buffer[43] << 8) +
				buffer[44];

			break;

	}

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

	if (pcontrols[C_RIGHT]) {

		// Walk/run right

		if (dx < 0) dx += PXA_REVERSE * mspf;
		else if (dx < PXS_WALK) dx += PXA_WALK * mspf;
		else if (dx < PXS_RUN) dx += PXA_RUN * mspf;

		facing = true;

	} else if (pcontrols[C_LEFT]) {

		// Walk/run left

		if (dx > 0) dx -= PXA_REVERSE * mspf;
		else if (dx > -PXS_WALK) dx -= PXA_WALK * mspf;
		else if (dx > -PXS_RUN) dx -= PXA_RUN * mspf;

		facing = false;

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
			else if (dy > -PXS_WALK) dy -= PXA_WALK * mspf;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * mspf;

		} else if (pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += PXA_REVERSE * mspf;
			else if (dy < PXS_WALK) dy += PXA_WALK * mspf;
			else if (dy < PXS_RUN) dy += PXA_RUN * mspf;

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

	} else if (y > level->getWaterLevel(ticks)) {

		if (pcontrols[C_JUMP]) {

			// Swim upwards

			if (dy > 0) dy -= PXA_REVERSE * mspf;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * mspf;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * mspf;

			// Prepare to jump upon leaving the water

			if (!level->checkMask(x + PXO_MID, y - F36)) {

				jumpY = y - jumpHeight;

				if (dx < 0) jumpY += dx >> 4;
				else if (dx > 0) jumpY -= dx >> 4;

				event = NULL;

			}

		} else if (pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += PXA_REVERSE * mspf;
			else if (dy < PXS_WALK) dy += PXA_WALK * mspf;
			else if (dy < PXS_RUN) dy += PXA_RUN * mspf;

		} else {

			// Sink

			dy += PYA_SINK * mspf;
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
				if (dx < 0) jumpY += dx >> 4;
				else if (dx > 0) jumpY -= dx >> 4;

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
			(event[E_BEHAVIOUR] != 25)))) jumpY = LH * F32;

		// If jumping, rise
		if (y >= jumpY) {

			dy = (jumpY - y - F64) * 4;

			// Avoid jumping to fast, unless caused by an event
			if (!event && (dy < ((-92 * F1) - F64) * 4))
				dy = ((-92 * F1) - F64) * 4;

		} else {

			// Fall under gravity
			dy += PYA_GRAVITY * mspf;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		// Stop looking
		if (!pcontrols[C_UP] && !pcontrols[C_DOWN]) lookTime = 0;

	}

	// If there is an obstacle above and the player is not floating up, stop
	// rising
	if (level->checkMask(x + PXO_MID, y + PYO_TOP - F4) && (jumpY < y) &&
		(!event || event[E_BEHAVIOUR] != 25)) {

		jumpY = LH * F32;
		if (dy < 0) dy = 0;

		if (event && (event[E_MODIFIER] != 6) && (event[E_BEHAVIOUR] != 28))
			event = NULL;

	}

	// If jump completed, stop rising
	if (y <= jumpY) {

		jumpY = LH * F32;

		if (event && (event[E_MODIFIER] != 6) && (event[E_BEHAVIOUR] != 28))
			event = NULL;

	}


	// Handle firing
	if (pcontrols[C_FIRE]) {

		if (ticks > fireTime) {

			// Create new bullet
			level->firstBullet =
				new Bullet(this, false, ticks, level->firstBullet);

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

		releaseControl(C_CHANGE);

		ammoType = ((ammoType + 2) % 5) - 1;

		// If there is no ammo of this type, go to the next type that has ammo
		while ((ammoType > -1) && !ammo[ammoType])
			ammoType = ((ammoType + 2) % 5) - 1;

	}


	// Deal with the bird

	if (bird) {

		if (bird->playFrame(ticks)) {

			delete bird;
			bird = NULL;

		}

	}


	return;

}


void Player::move (int ticks) {

	fixed pdx, pdy;
	int count;

	if (warpTime && (ticks > warpTime)) {

		x = warpX << 15;
		y = (warpY + 1) << 15;
		warpTime = 0;

	}

	// Apply as much of the trajectory as possible, without going into the
	// scenery

	if (fastFeetTime > ticks) {

		pdx = (dx * mspf * 3) >> 11;
		pdy = (dy * mspf * 3) >> 11;

	} else {

		pdx = (dx * mspf) >> 10;
		pdy = (dy * mspf) >> 10;

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

		jumpY = LH * F32;
		event = NULL;

	}


	// If the player has hit the bottom of the level, kill
	if (y + F4 > (LH * TH << 10)) kill(ticks);


	// Handle spikes
	if (level->checkSpikes(x + PXO_MID, y + PYO_TOP - F4) ||
		level->checkSpikes(x + PXO_MID, y + F4) ||
		level->checkSpikes(x + PXO_L - F4, y + PYO_MID) ||
		level->checkSpikes(x + PXO_R + F4, y + PYO_MID)) hit(ticks);


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

	viewX = x + F8 - (viewW << 9);

	if (!lookTime || (ticks < 1000 + lookTime) || (ticks < 1000 - lookTime)) {

		viewY = y - F24 - (viewH << 9);

	} else if (lookTime > 0) {

		if (ticks < 2000 + lookTime)
			viewY = y - F24 - (64 * (lookTime + 1000 - ticks)) - (viewH << 9);
		else viewY = y + F64 - F24 - (viewH << 9);

	} else {

		if (ticks < 2000 - lookTime)
			viewY = y - F24 - (64 * (lookTime - 1000 + ticks)) - (viewH << 9);
		else viewY = y - F64 - F24 - (viewH << 9);

	}


	// Apply lag proportional to player "speed"
	speed = ((dx >= 0? dx: -dx) + (dy >= 0? dy: -dy)) >> 14;

	if (mspf < speed) {

		viewX = ((oldViewX * (speed - mspf)) + (viewX * mspf)) / speed;
		viewY = ((oldViewY * (speed - mspf)) + (viewY * mspf)) / speed;

	}


	return;

}

void Player::draw (int ticks) {

	Anim *an;
	int anim, frame;
	fixed xOffset, yOffset;

	// The current frame for animations
	if (reaction == PR_KILLED) frame = (ticks + PRT_KILLED - reactionTime) / 75;
	else frame = ticks / 75;


	// Choose player animation

	if (reaction == PR_KILLED) anim = anims[facing? PA_RDIE: PA_LDIE];

	else if ((reaction == PR_HURT) &&
		(reactionTime - ticks > PRT_HURT - PRT_HURTANIM))
		anim = anims[facing? PA_RHURT: PA_LHURT];

	else if (y > level->getWaterLevel(ticks))
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
	if (fastFeetTime > ticks)
		an->draw(x - (dx >> 6), y);

	// Draw player
	an->draw(x, y);


	// Remove red flash or player colour from sprite
	an->restorePalette();


	if (reaction == PR_INVINCIBLE) {

		// Show invincibility stars

		xOffset = (int)(sin(ticks / 100.0f) * F12);
		yOffset = (int)(cos(ticks / 100.0f) * F12);

		an = level->getMiscAnim(0);

		an->setFrame(frame, true);
		an->draw(x + PXO_MID + xOffset, y + PYO_MID + yOffset);

		an->setFrame(frame + 1, true);
		an->draw(x + PXO_MID - xOffset, y + PYO_MID - yOffset);

		an->setFrame(frame + 2, true);
		an->draw(x + PXO_MID + yOffset, y + PYO_MID + xOffset);

		an->setFrame(frame + 3, true);
		an->draw(x + PXO_MID - yOffset, y + PYO_MID - xOffset);

	} else if (shield > 2) {

		// Show the 4-hit shield

		xOffset = (int)(cos(ticks / 200.0f) * F20);
		yOffset = (int)(sin(ticks / 200.0f) * F20);

		an = level->getAnim(59);

		an->draw(x + xOffset, y + PYO_TOP + yOffset);

		if (shield > 3) an->draw(x - xOffset, y + PYO_TOP - yOffset);

		if (shield > 4) an->draw(x + yOffset, y + PYO_TOP - xOffset);

		if (shield > 5) an->draw(x - yOffset, y + PYO_TOP + xOffset);

	} else if (shield) {

		// Show the 2-hit shield

		xOffset = (int)(cos(ticks / 200.0f) * F20);
		yOffset = (int)(sin(ticks / 200.0f) * F20);

		an = level->getAnim(50);

		an->draw(x + xOffset, y + yOffset + PYO_TOP);

		if (shield == 2) an->draw(x - xOffset, y + PYO_TOP - yOffset);

	}


	// Show the bird
	if (bird) bird->draw(ticks);


	// Show the player's name
	if (game->getMode() != M_SINGLE)
		panelBigFont->showString(name, (x - viewX) >> 10,
			(y - F32 - F16 - viewY) >> 10);

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


