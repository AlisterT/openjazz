
/*
 *
 * player.cpp
 *
 * 3rd February 2009: Created player.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
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
 * Deals with the creation and destruction of players, and their interactions
 * with other objects.
 *
 */


#include "bird.h"
#include "player.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "level/event/event.h"
#include "level/level.h"

#include <string.h>


Player::Player () {

	bird = NULL;

	name = NULL;

	return;

}


Player::~Player () {

	deinit();

	return;

}


void Player::init (char *playerName, unsigned char *playerCols, unsigned char newTeam) {

	int offsets[15] = {PC_WHITE, PC_SGREEN, PC_BLUE, PC_RED, PC_LGREEN,
		PC_LEVEL1, PC_YELLOW, PC_LEVEL2, PC_ORANGE, PC_LEVEL3, PC_LEVEL4,
		PC_SANIM, PC_LANIM, PC_LEVEL5, 256};
	int count, start, length;

	// Clear existing player
	deinit();

	// Assign name
	name = createString(playerName);

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

	event = 0;

	for (count = 0; count < PCONTROLS; count++) pcontrols[count] = false;

	energy = 4;
	shield = 0;
	floating = false;
	facing = true;
	direction = FQ;
	reaction = PR_NONE;
	reactionTime = 0;
	jumpHeight = ITOF(92);
	jumpY = TTOF(LH);
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


bool Player::takeEvent (unsigned char gridX, unsigned char gridY, unsigned int ticks) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	switch (set[E_MODIFIER]) {

		case 41: // Bonus level

			if (getEnergy()) level->setNext(set[E_MULTIPURPOSE], set[E_YAXIS]);

			// The lack of a break statement is intentional

		case 8: // Boss
		case 27: // End of level

			if (!getEnergy()) return false;

			if (!gameMode) {

				if (game) game->setCheckpoint(gridX, gridY);

				level->setStage(LS_END);

			} else if (!(gameMode->endOfLevel(this, gridX, gridY))) return false;

			break;

		case 0: // Enemy

			break;

		case 1: // Invincibility

			if (!getEnergy()) return false;

			reaction = PR_INVINCIBLE;
			reactionTime = ticks + PRT_INVINCIBLE;

			break;

		case 2:
		case 3: // Health

			if (energy < 4) energy++;

			break;

		case 4: // Extra life

			if (lives < 99) lives++;

			break;

		case 5: // High-jump feet

			jumpHeight += F16;

			break;

		case 7: // Used with destructible blocks

			break;

		case 9: // Sand timer

			level->addTimer();

			break;

		case 10: // Checkpoint

			if (game) game->setCheckpoint(gridX, gridY);

			break;

		case 11: // Item

			break;

		case 12: // Rapid fire

			fireSpeed++;

			break;

		case 15: // Ammo

			addAmmo(0, 15);

			break;

		case 16: // Ammo

			addAmmo(1, 15);

			break;

		case 17: // Ammo

			addAmmo(2, 15);

			break;

		case 18: // Ammo

			addAmmo(0, 2);

			break;

		case 19: // Ammo

			addAmmo(1, 2);

			break;

		case 20: // Ammo

			addAmmo(2, 2);

			break;

		case 26: // Fast feet box

			fastFeetTime = ticks + T_FASTFEET;

			break;

		case 30: // TNT

			addAmmo(3, 1);

			break;

		case 31: // Water level

			level->setWaterLevel(gridY);

			break;

		case 33: // 2-hit shield

			if (shield < 2) shield = 2;

			break;

		case 34: // Bird

			if (!bird) bird = new Bird(this, gridX, gridY);

			break;

		case 35: // Airboard, etc.

			floating = true;

			break;

		case 36: // 4-hit shield

			shield = 6;

			break;

		case 37: // Diamond

			if (game) game->setBonus(0);

			// Yellow flash
			firstPE = new FlashPaletteEffect(255, 255, 0, 320, firstPE);

			break;

		default:

			return false;

	}

	addScore(set[E_ADDEDSCORE]);

	// Add to player's enemy/item tally
	// If the event hurts and can be killed, it is an enemy
	// Anything else that scores is an item
	if ((set[E_MODIFIER] == 0) && set[E_HITSTOKILL]) enemies++;
	else if (set[E_ADDEDSCORE]) items++;

	return true;

}


bool Player::touchEvent (unsigned char gridX, unsigned char gridY, unsigned int ticks, int msps) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	switch (set[E_MODIFIER]) {

		case 0: // Hurt
		case 8: // Boss

			if ((set[E_BEHAVIOUR] < 37) || (set[E_BEHAVIOUR] > 44))
				hit(NULL, ticks);

			break;

		case 7: // Used with destructible blocks, but should not destroy on contact

			break;

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

		case 28: // Belt

			x += set[E_MAGNITUDE] * 4 * msps;

			break;

		case 29: // Upwards spring

			setEvent(gridX, gridY);

			level->playSound(set[E_SOUND]);

			break;

		case 31: // Water level

			level->setWaterLevel(gridY);

			break;

		case 32: // Float up / sideways

			if (set[E_YAXIS]) {

				eventX = gridX;
				eventY = gridY;
				event = 2;

				if (dy > set[E_MULTIPURPOSE] * -F20)
					dy -= set[E_MULTIPURPOSE] * 320 * msps;

				jumpY = y - (8 * F16);

			} else if (set[E_MAGNITUDE] < 0) {

				if (!level->checkMaskDown(x + PXO_L + (set[E_MAGNITUDE] * 20 * msps), y + PYO_MID))
					x += set[E_MAGNITUDE] * 20 * msps;

			} else {

				if (!level->checkMaskDown(x + PXO_R + (set[E_MAGNITUDE] * 20 * msps), y + PYO_MID))
					x += set[E_MAGNITUDE] * 20 * msps;

			}

			break;

		case 38: // Airboard, etc. off

			floating = false;

			break;

		default:

			if (!set[E_HITSTOKILL]) return takeEvent(gridX, gridY, ticks);

			break;

	}

	return false;

}


bool Player::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == team)) return false;


	if (shield == 3) shield = 0;
	else if (shield) shield--;
	else if (!gameMode || gameMode->hit(source, this)) {

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

		kill(source, ticks);

	}

	return true;

}


void Player::kill (Player *source, unsigned int ticks) {

	if (reaction != PR_NONE) return;

	if (!gameMode || gameMode->kill(source, this)) {

		energy = 0;
		lives--;

		reaction = PR_KILLED;
		reactionTime = ticks + PRT_KILLED;

	}

	if (!gameMode) firstPE = new FadeOutPaletteEffect(T_END, firstPE);

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


bool Player::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + PXO_R >= left) && (x + PXO_L < left + width) &&
		(y >= top) && (y + PYO_TOP < top + height);

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


fixed Player::getDirection () {

	return direction;

}


Anim * Player::getAnim () {

	return level->getAnim(anims[animType]);

}


unsigned char Player::getTeam () {

	return team;

}


void Player::setEvent (unsigned char gridX, unsigned char gridY) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	if (set[E_MODIFIER] == 29) {

		// Upwards spring
		jumpY = y + (set[E_MAGNITUDE] * (F20 + F1));
		event = 1;

	} else if (set[E_MODIFIER] == 6) event = 3;
	else if (set[E_BEHAVIOUR] == 28) event = 4;
	else return;

	eventX = gridX;
	eventY = gridY;

	return;

}


void Player::clearEvent (unsigned char gridX, unsigned char gridY) {

	// If the location matches, clear the event

	if ((gridX == eventX) && (gridY == eventY)) event = 0;

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
	data[45] = pcontrols[C_SWIM];

	return;

}


void Player::receive (unsigned char *buffer) {

	// Interpret data received from client/server

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
			pcontrols[C_SWIM] = buffer[45];
			pcontrols[C_FIRE] = buffer[8];
			pcontrols[C_CHANGE] = false;

			if ((buffer[9] & 1) && !bird)
				bird = new Bird(this, FTOT(x), FTOT(y));

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


PlayerReaction Player::reacted (unsigned int ticks) {

	PlayerReaction oldReaction;

	if ((reaction != PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = PR_NONE;

		return oldReaction;

	}

	return PR_NONE;

}


