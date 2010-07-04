
/*
 *
 * levelplayer.cpp
 *
 * 24th June 2010: Created levelplayer.cpp from parts of player.cpp
 * 29th June 2010: Created jj2levelplayer.cpp from parts of levelplayer.cpp
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
 * Deals with the creation and destruction of players in levels, and their
 * interactions with other level objects.
 *
 */


#include "bird.h"
#include "levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/sound.h"
#include "level/event/event.h"
#include "level/level.h"

#include <string.h>


LevelPlayer::LevelPlayer (Player* parent, char* newAnims, unsigned char startX, unsigned char startY, bool hasBird) {

	int offsets[15] = {PCO_GREY, PCO_SGREEN, PCO_BLUE, PCO_RED, PCO_LGREEN,
		PCO_LEVEL1, PCO_YELLOW, PCO_LEVEL2, PCO_ORANGE, PCO_LEVEL3, PCO_LEVEL4,
		PCO_SANIM, PCO_LANIM, PCO_LEVEL5, 256};
	int count, start, length;


	player = parent;

	if (newAnims) memcpy(anims, newAnims, PANIMS);
	else memset(anims, 0, PANIMS);

	if (hasBird) bird = new Bird(this, startX, startY - 2);
	else bird = NULL;

	shield = 0;
	enemies = items = 0;
	gem = false;

	reset(startX, startY);


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;


	// Fur colours

	start = offsets[player->cols[0]];
	length = offsets[player->cols[0] + 1] - start;

	for (count = 0; count < 16; count++)
		palette[count + 48].r = palette[count + 48].g = palette[count + 48].b =
			(count * length / 16) + start;


	// Bandana colours

	start = offsets[player->cols[1]];
	length = offsets[player->cols[1] + 1] - start;

	for (count = 0; count < 16; count++)
		palette[count + 32].r = palette[count + 32].g = palette[count + 32].b =
 			(count * length / 16) + start;


	// Gun colours

	start = offsets[player->cols[2]];
	length = offsets[player->cols[2] + 1] - start;

	for (count = 0; count < 9; count++)
		palette[count + 23].r = palette[count + 23].g = palette[count + 23].b =
			(count * length / 9) + start;


	// Wristband colours

	start = offsets[player->cols[3]];
	length = offsets[player->cols[3] + 1] - start;

	for (count = 0; count < 8; count++)
		palette[count + 88].r = palette[count + 88].g = palette[count + 88].b =
			(count * length / 8) + start;


	return;

}


LevelPlayer::~LevelPlayer () {

	if (bird) delete bird;

	return;

}


void LevelPlayer::reset (unsigned char startX, unsigned char startY) {

	event = LPE_NONE;
	energy = 4;
	floating = false;
	facing = true;
	animType = PA_RSTAND;
	reaction = PR_NONE;
	reactionTime = 0;
	jumpHeight = ITOF(92);
	jumpY = TTOF(LH);
	fastFeetTime = 0;
	warpTime = 0;
	dx = 0;
	dy = 0;
	x = TTOF(startX);
	y = TTOF(startY);

	return;

}


void LevelPlayer::addItem () {

	items++;

	return;

}


void LevelPlayer::clearEvent (unsigned char gridX, unsigned char gridY) {

	// If the location matches, clear the event

	if ((gridX == eventX) && (gridY == eventY)) event = LPE_NONE;

	return;

}


unsigned char LevelPlayer::getAnim () {

	return anims[animType];

}


int LevelPlayer::getEnemies () {

	return enemies;

}


int LevelPlayer::getEnergy () {

	return energy;

}


bool LevelPlayer::getFacing () {

	return facing;

}


int LevelPlayer::getItems () {

	return items;

}


bool LevelPlayer::hasBird () {

	return bird;

}


bool LevelPlayer::hasGem () {

	return gem;

}


bool LevelPlayer::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == player->team)) return false;


	if (shield == 3) shield = 0;
	else if (shield) shield--;
	else if (!gameMode || gameMode->hit(source, player)) {

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


void LevelPlayer::kill (Player *source, unsigned int ticks) {

	if (reaction != PR_NONE) return;

	if (!gameMode || gameMode->kill(source, player)) {

		energy = 0;
		player->lives--;

		reaction = PR_KILLED;
		reactionTime = ticks + PRT_KILLED;

	}

	if (!gameMode) level->flash(0, 0, 0, T_END << 1);

	return;

}


bool LevelPlayer::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + PXO_R >= left) && (x + PXO_L < left + width) &&
		(y >= top) && (y + PYO_TOP < top + height);

}


PlayerReaction LevelPlayer::reacted (unsigned int ticks) {

	PlayerReaction oldReaction;

	if ((reaction != PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = PR_NONE;

		return oldReaction;

	}

	return PR_NONE;

}


void LevelPlayer::setEvent (unsigned char gridX, unsigned char gridY) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	if (set[E_MODIFIER] == 29) {

		// Upwards spring
		jumpY = y + (set[E_MAGNITUDE] * (F20 + F1));
		event = LPE_SPRING;

	} else if (set[E_MODIFIER] == 6) event = LPE_PLATFORM;
	else if (set[E_BEHAVIOUR] == 28) event = LPE_PLATFORM;
	else return;

	eventX = gridX;
	eventY = gridY;

	return;

}


void LevelPlayer::setPosition (fixed newX, fixed newY) {

	x = newX;
	y = newY;

	return;

}


void LevelPlayer::setSpeed (fixed newDx, fixed newDy) {

	dx = newDx;
	if (newDy) dy = newDy;

	return;

}


bool LevelPlayer::takeEvent (unsigned char gridX, unsigned char gridY, unsigned int ticks) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	switch (set[E_MODIFIER]) {

		case 41: // Bonus level

			if (energy) level->setNext(set[E_MULTIPURPOSE], set[E_YAXIS]);

			// The lack of a break statement is intentional

		case 8: // Boss
		case 27: // End of level

			if (!energy) return false;

			if (!gameMode) {

				if (game) game->setCheckpoint(gridX, gridY);

				level->setStage(LS_END);

			} else if (!(gameMode->endOfLevel(player, gridX, gridY))) return false;

			break;

		case 0: // Enemy

			break;

		case 1: // Invincibility

			if (!energy) return false;

			reaction = PR_INVINCIBLE;
			reactionTime = ticks + PRT_INVINCIBLE;

			break;

		case 2:
		case 3: // Health

			if (energy < 4) energy++;

			break;

		case 4: // Extra life

			player->addLife();

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

			player->fireSpeed++;

			break;

		case 15: // Ammo

			player->addAmmo(0, 15);

			break;

		case 16: // Ammo

			player->addAmmo(1, 15);

			break;

		case 17: // Ammo

			player->addAmmo(2, 15);

			break;

		case 18: // Ammo

			player->addAmmo(0, 2);

			break;

		case 19: // Ammo

			player->addAmmo(1, 2);

			break;

		case 20: // Ammo

			player->addAmmo(2, 2);

			break;

		case 26: // Fast feet box

			fastFeetTime = ticks + T_FASTFEET;

			break;

		case 30: // TNT

			player->addAmmo(3, 1);

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

			gem = true;

			// Yellow flash
			level->flash(255, 255, 0, 320);

			break;

		default:

			return false;

	}

	player->addScore(set[E_ADDEDSCORE] * 10);

	// Add to player's enemy/item tally
	// If the event hurts and can be killed, it is an enemy
	// Anything else that scores is an item
	if ((set[E_MODIFIER] == 0) && set[E_HITSTOKILL]) enemies++;
	else if (set[E_ADDEDSCORE]) items++;

	return true;

}


bool LevelPlayer::touchEvent (unsigned char gridX, unsigned char gridY, unsigned int ticks, int msps) {

	signed char *set;

	set = level->getEvent(gridX, gridY);

	switch (set[E_MODIFIER]) {

		case 0: // Hurt
		case 8: // Boss

			if ((set[E_BEHAVIOUR] < 37) || (set[E_BEHAVIOUR] > 44)) hit(NULL, ticks);

			break;

		case 7: // Used with destructible blocks, but should not destroy on contact

			break;

		case 13: // Warp

			if (!warpTime) {

				warpX = set[E_MULTIPURPOSE];
				warpY = set[E_YAXIS];
				warpTime = ticks + T_WARP;

				// White flash
				level->flash(255, 255, 255, T_WARP);

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
				event = LPE_FLOAT;

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


void LevelPlayer::send (unsigned char *buffer) {

	// Copy data to be sent to clients/server

	buffer[9] = bird? 1: 0;
	buffer[23] = energy;
	buffer[25] = shield;
	buffer[26] = floating;
	buffer[27] = getFacing();
	buffer[29] = jumpHeight >> 24;
	buffer[30] = (jumpHeight >> 16) & 255;
	buffer[31] = (jumpHeight >> 8) & 255;
	buffer[32] = jumpHeight & 255;
	buffer[33] = jumpY >> 24;
	buffer[34] = (jumpY >> 16) & 255;
	buffer[35] = (jumpY >> 8) & 255;
	buffer[36] = jumpY & 255;
	buffer[37] = x >> 24;
	buffer[38] = (x >> 16) & 255;
	buffer[39] = (x >> 8) & 255;
	buffer[40] = x & 255;
	buffer[41] = y >> 24;
	buffer[42] = (y >> 16) & 255;
	buffer[43] = (y >> 8) & 255;
	buffer[44] = y & 255;

	return;

}


void LevelPlayer::receive (unsigned char *buffer) {

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_P_ANIMS:

			memcpy(anims, (char *)buffer + 3, PANIMS);

			break;

		case MT_P_TEMP:

			if ((buffer[9] & 1) && !bird) bird = new Bird(this, FTOT(x), FTOT(y) - 2);

			if (!(buffer[9] & 1) && bird) {

				delete bird;
				bird = NULL;

			}

			energy = buffer[23];
			shield = buffer[25];
			floating = buffer[26];
			facing = buffer[27];
			jumpHeight = (buffer[29] << 24) + (buffer[30] << 16) + (buffer[31] << 8) + buffer[32];
			jumpY = (buffer[33] << 24) + (buffer[34] << 16) + (buffer[35] << 8) + buffer[36];
			x = (buffer[37] << 24) + (buffer[38] << 16) + (buffer[39] << 8) + buffer[40];
			y = (buffer[41] << 24) + (buffer[42] << 16) + (buffer[43] << 8) + buffer[44];

			break;

	}

	return;

}

