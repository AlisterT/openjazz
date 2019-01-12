
/**
 *
 * @file jj1bonuslevelplayer.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * - 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * - 18th July 2009: Created playerframe.cpp from parts of player.cpp
 * - 24th June 2010: Created bonusplayer.cpp from parts of player.cpp and
 *                 playerframe.cpp
 * - 1st August 2012: Renamed bonusplayer.cpp to jj1bonuslevelplayer.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with players in bonus levels.
 *
 */


#include "../jj1bonuslevel.h"
#include "jj1bonuslevelplayer.h"

#include "game/game.h"
#include "io/controls.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "util.h"

#include <string.h>


/**
 * Create a JJ1 bonus level player.
 *
 * @param parent The game player corresponding to this level player.
 * @param newAnims Animations
 * @param startX Starting position x-coordinate
 * @param startY Starting position y-coordinate
 * @param flockSize The number of birds accompanying the player
 */
JJ1BonusLevelPlayer::JJ1BonusLevelPlayer (Player* parent, Anim **newAnims, unsigned char startX, unsigned char startY, int flockSize) {

	int count;


	player = parent;

	memcpy(anims, newAnims, BPANIMS * sizeof(Anim*));

	birds = flockSize;

	reset(startX, startY);


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;

	/// @todo Custom colours


	return;

}


/**
 * Delete the JJ1 bonus level player.
 */
JJ1BonusLevelPlayer::~JJ1BonusLevelPlayer () {

	return;

}


/**
 * Reset the player's position, energy etc.
 *
 * @param startX New x-coordinate
 * @param startY New y-coordinate
 */
void JJ1BonusLevelPlayer::reset (int startX, int startY) {

	x = TTOF(startX) + F16;
	y = TTOF(startY) + F16;
	z = 0;
	dx = 0;
	dy = 0;
	dz = 0;

	direction = FQ;
	dr = 0;
	da = 0;

	gems = 0;

	animType = PA_WALK;

	return;

}


/**
 * Add to the player's gem tally.
 */
void JJ1BonusLevelPlayer::addGem () {

	gems++;

	return;

}


/**
 * Determine the direction the player is facing.
 *
 * @return The player's direction
 */
fixed JJ1BonusLevelPlayer::getDirection () {

	return direction;

}


/**
 * Determine the number of gems the player has collected.
 *
 * @return Number of gems collected
 */
int JJ1BonusLevelPlayer::getGems () {

	return gems;

}


/**
 * Determine the player's Z-coordinate (altitude).
 *
 * @return Player's Z-coordinate
 */
fixed JJ1BonusLevelPlayer::getZ () {

	return z;

}


/**
 * Determine how many birds are accompanying the player.
 *
 * @return The number of birds accompanying the player
 */
int JJ1BonusLevelPlayer::countBirds () {

	return birds;

}


/**
 * Player iteration.
 *
 * @param ticks Time
 * @param msps Ticks per step
 * @param bonus Bonus level
 */
void JJ1BonusLevelPlayer::step (unsigned int ticks, int msps, JJ1BonusLevel* bonus) {

	(void)ticks;

	fixed cdx, cdy;

	// Bonus stages use polar coordinates for movement (but not position)

	if (animType == PA_CRASH) {

		if (dr < 0) dr += PRA_REBOUND * msps;
		else {

			dr = 0;

			animType = PA_OTHER;

		}

	} else {

		if (player->getControl(C_FIRE)) {

			running = true;
			dr = PRS_RUN;

		}

		if (player->getControl(C_UP)) {

			// Walk/run forwards

			if (dr < 0) dr += PRA_REVERSE * msps;
			else if (dr < PRS_WALK) dr += PRA_WALK * msps;

			animType = PA_WALK;

		} else if (player->getControl(C_DOWN)) {

			// Walk/run back

			running = false;

			if (dr > 0) dr -= PRA_REVERSE * msps;
			else if (dr > PRS_REVERSE) dr -= PRA_WALK * msps;

			animType = PA_WALK;

		} else {

			// Slow down

			if (!running && (dr > 0)) {

				if (dr < PRA_STOP * msps) dr = 0;
				else dr -= PRA_STOP * msps;

			}

			if (dr < 0) {

				if (dr > -PRA_STOP * msps) dr = 0;
				else dr += PRA_STOP * msps;

			}

			if (dr == 0) running = false;

			animType = PA_OTHER;

		}

		if (player->getControl(C_LEFT)) {

			if (da > -PAS_TURN) da -= PAA_TURN * msps;

			animType = PA_LEFT;

		} else if (player->getControl(C_RIGHT)) {

			if (da < PAS_TURN) da += PAA_TURN * msps;

			animType = PA_RIGHT;

		} else {

			// Slow down rotation

			if (da > 0) {

				if (da < PAA_STOP * msps) da = 0;
				else da -= PAA_STOP * msps;

			}

			if (da < 0) {

				if (da > -PAA_STOP * msps) da = 0;
				else da += PAA_STOP * msps;

			}

		}

		if ((z == 0) && player->getControl(C_JUMP)) {

			// Jump

			dz = PZS_JUMP;

			if (dr < PRS_JUMP) dr = PRS_JUMP;

		}

		if (dz > PZS_FALL) dz += PZA_GRAVITY * msps;

		if (z > 0) animType = PA_JUMP;

	}


	// Apply trajectory

	direction += (da * msps) >> 10;

	cdx = (MUL(fSin(direction), dr) * msps) >> 10;
	cdy = (MUL(-fCos(direction), dr) * msps) >> 10;

	if (!bonus->checkMask(x + cdx, y)) x += cdx;
	if (!bonus->checkMask(x, y + cdy)) y += cdy;

	z += (dz * msps) >> 10;
	if (z > F1) z = F1;
	if (z < 0) z = 0;


	// React to running collision

	if (running && bonus->checkMask(x + cdx, y + cdy)) {

		running = false;
		dr = PRS_CRASH;
		da = 0;

		animType = PA_CRASH;

	}


	return;

}


/**
 * Draw the player.
 *
 * @param ticks Time
 */
void JJ1BonusLevelPlayer::draw (unsigned int ticks) {

	Anim* anim;

	anim = anims[animType];
	anim->setFrame(ticks / 75, true);
	if (canvasW <= SW) anim->draw(ITOF((canvasW - anim->getWidth()) >> 1), ITOF(canvasH - anim->getHeight() - 16 - FTOI(z * 80)));
	else anim->drawScaled(ITOF(canvasW >> 1), ITOF(canvasH - ((((anim->getHeight() >> 1) + 16 + FTOI(z * 80)) * canvasW) / SW)), ITOF(canvasW) / SW);

	return;

}


/**
 * Fill a buffer with player data.
 *
 * @param buffer The buffer
 */
void JJ1BonusLevelPlayer::send (unsigned char *buffer) {

	// Copy data to be sent to clients/server

	buffer[9] = birds;
	buffer[23] = 0;
	buffer[25] = 0;
	buffer[26] = 0;
	buffer[27] = direction >> 2;
	buffer[29] = 0;
	buffer[30] = 0;
	buffer[31] = 0;
	buffer[32] = 0;
	buffer[33] = z >> 24;
	buffer[34] = (z >> 16) & 255;
	buffer[35] = (z >> 8) & 255;
	buffer[36] = z & 255;
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


/**
 * Adjust player data based on the contents of a given buffer.
 *
 * @param buffer The buffer
 */
void JJ1BonusLevelPlayer::receive (unsigned char *buffer) {

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_P_TEMP:

			birds = buffer[9];
			direction = buffer[27] << 2;
			z = (buffer[33] << 24) + (buffer[34] << 16) + (buffer[35] << 8) + buffer[36];
			x = (buffer[37] << 24) + (buffer[38] << 16) + (buffer[39] << 8) + buffer[40];
			y = (buffer[41] << 24) + (buffer[42] << 16) + (buffer[43] << 8) + buffer[44];

			break;

	}

	return;

}

