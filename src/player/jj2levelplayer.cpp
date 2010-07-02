
/*
 *
 * jj2levelplayer.cpp
 *
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


#include "jj2levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/sound.h"
#include "jj2level/jj2event/jj2event.h"
#include "jj2level/jj2level.h"

#include <string.h>


JJ2LevelPlayer::JJ2LevelPlayer (Player* parent, char* newAnims, unsigned char startX, unsigned char startY, bool hasBird) {

	int offsets[14] = {JJ2PCO_GREY, JJ2PCO_SGREEN, JJ2PCO_BLUE, JJ2PCO_RED,
		JJ2PCO_LGREEN, JJ2PCO_LEVEL1, JJ2PCO_YELLOW, JJ2PCO_LEVEL2,
		JJ2PCO_ORANGE, JJ2PCO_LEVEL3, JJ2PCO_LEVEL4, JJ2PCO_SANIM, JJ2PCO_LANIM,
		JJ2PCO_LEVEL5};
	int lengths[14] = {JJ2PCL_GREY, JJ2PCL_SGREEN, JJ2PCL_BLUE, JJ2PCL_RED,
		JJ2PCL_LGREEN, JJ2PCL_LEVEL1, JJ2PCL_YELLOW, JJ2PCL_LEVEL2,
		JJ2PCL_ORANGE, JJ2PCL_LEVEL3, JJ2PCL_LEVEL4, JJ2PCL_SANIM, JJ2PCL_LANIM,
		JJ2PCL_LEVEL5};
	int count, start, length;


	player = parent;

	if (newAnims) memcpy(anims, newAnims, PANIMS);
	else memset(anims, 0, PANIMS);

	bird = hasBird;

	shield = JJ2S_NONE;

	reset(startX, startY);


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;


	// Fur colours

	start = offsets[player->cols[0]];
	length = lengths[player->cols[0]];

	for (count = 0; count < 16; count++)
		palette[count + 48].r = palette[count + 48].g = palette[count + 48].b =
			(count * length / 16) + start;


	// Bandana colours

	start = offsets[player->cols[1]];
	length = lengths[player->cols[1]];

	for (count = 0; count < 16; count++)
		palette[count + 32].r = palette[count + 32].g = palette[count + 32].b =
 			(count * length / 16) + start;


	// Gun colours

	start = offsets[player->cols[2]];
	length = lengths[player->cols[2]];

	for (count = 0; count < 9; count++)
		palette[count + 23].r = palette[count + 23].g = palette[count + 23].b =
			(count * length / 9) + start;


	// Wristband colours

	start = offsets[player->cols[3]];
	length = lengths[player->cols[3]];

	for (count = 0; count < 8; count++)
		palette[count + 88].r = palette[count + 88].g = palette[count + 88].b =
			(count * length / 8) + start;


	// Fix creepy black eyes

	for (count = 0; count < 16; count++)
		palette[count].r = palette[count].g = palette[count].b = (count >> 1) + 64;


	return;

}


JJ2LevelPlayer::~JJ2LevelPlayer () {

	return;

}


void JJ2LevelPlayer::reset (unsigned char startX, unsigned char startY) {

	event = NULL;
	mod = NULL;
	energy = 5;
	floating = false;
	facing = true;
	animType = PA_RSTAND;
	reaction = JJ2PR_NONE;
	reactionTime = 0;
	jumpHeight = ITOF(92);
	jumpY = TTOF(256);
	fastFeetTime = 0;
	stopTime = 0;
	dx = 0;
	dy = 0;
	x = TTOF(startX);
	y = TTOF(startY);
	gems[0] = gems[1] = gems[2] = gems[3] = 0;

	return;

}


void JJ2LevelPlayer::addGem (int colour) {

	gems[colour]++;

	return;

}


unsigned char JJ2LevelPlayer::getAnim () {

	return anims[animType];

}


int JJ2LevelPlayer::getEnergy () {

	return energy;

}


bool JJ2LevelPlayer::getFacing () {

	return facing;

}


int JJ2LevelPlayer::getGems (int colour) {

	return gems[colour];

}


bool JJ2LevelPlayer::hasBird () {

	return bird;

}


bool JJ2LevelPlayer::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != JJ2PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == player->team)) return false;


	if (!gameMode || gameMode->hit(source, player)) {

		energy--;

		//if (bird) bird->hit();

		playSound(S_OW);

	}

	if (energy) {

		reaction = JJ2PR_HURT;
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


void JJ2LevelPlayer::kill (Player *source, unsigned int ticks) {

	if (reaction != JJ2PR_NONE) return;

	if (!gameMode || gameMode->kill(source, player)) {

		energy = 0;
		player->lives--;

		reaction = JJ2PR_KILLED;
		reactionTime = ticks + PRT_KILLED;

	}

	return;

}


bool JJ2LevelPlayer::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + PXO_R >= left) && (x + PXO_L < left + width) &&
		(y >= top) && (y + PYO_TOP < top + height);

}


JJ2PlayerReaction JJ2LevelPlayer::reacted (unsigned int ticks) {

	JJ2PlayerReaction oldReaction;

	if ((reaction != JJ2PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = JJ2PR_NONE;

		return oldReaction;

	}

	return JJ2PR_NONE;

}


void JJ2LevelPlayer::setPosition (fixed newX, fixed newY) {

	x = newX;
	y = newY;

	return;

}


void JJ2LevelPlayer::setSpeed (fixed newDx, fixed newDy) {

	dx = newDx;
	if (newDy) dy = newDy;

	return;

}


bool JJ2LevelPlayer::takeEvent (JJ2Event* event, unsigned int ticks) {

	return true;

}


bool JJ2LevelPlayer::touchEvent (JJ2Event* touched, unsigned int ticks, int msps) {

	switch (touched->getType()) {

		case 60: // Frozen green spring

			jumpY = y - TTOF(14);
			event = touched;

			break;

		case 62: // Spring crate

			jumpY = y - TTOF(18);
			event = touched;

			break;

		case 83: // Checkpoint

			game->setCheckpoint(FTOT(x + PXO_MID), FTOT(y + PYO_MID));

			break;

		case 85: // Red spring

			jumpY = y - TTOF(7);
			event = touched;

			break;

		case 86: // Green spring

			jumpY = y - TTOF(14);
			event = touched;

			break;

		case 87: // Blue spring

			jumpY = y - TTOF(18);
			event = touched;

			break;

		default:

			break;

	}

	return false;

}


void JJ2LevelPlayer::send (unsigned char *buffer) {

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


void JJ2LevelPlayer::receive (unsigned char *buffer) {

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_P_ANIMS:

			memcpy(anims, (char *)buffer + 3, PANIMS);

			break;

		case MT_P_TEMP:

			if ((buffer[9] & 1) && !bird) bird = true;

			if (!(buffer[9] & 1) && bird) {

				bird = false;

			}

			energy = buffer[23];
			shield = (JJ2Shield)buffer[25];
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

