
/**
 *
 * @file jj2levelplayer.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 3rd February 2009: Renamed events.c to events.cpp and level.c to level.cpp,
 *                    created player.cpp
 * - 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * - 24th June 2010: Created levelplayer.cpp from parts of player.cpp
 * - 29th June 2010: Created jj2levelplayer.cpp from parts of levelplayer.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the creation and destruction of players in levels, and their
 * interactions with other level objects.
 *
 */


#include "../jj2event/jj2event.h"
#include "../jj2level.h"
#include "jj2levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/sound.h"

#include <string.h>


/**
 * Create a JJ2 level player.
 *
 * @param parent The game player corresponding to this level player.
 * @param newAnims Animations
 * @param newFlippedAnims Flipped animations
 * @param startX Starting position x-coordinate
 * @param startY Starting position y-coordinate
 * @param flockSize The number of birds accompanying the player
 */
JJ2LevelPlayer::JJ2LevelPlayer (Player* parent, Anim** newAnims,
	Anim** newFlippedAnims, unsigned char startX, unsigned char startY, int flockSize) {

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

	memcpy(anims, newAnims, JJ2PANIMS * sizeof(Anim *));
	memcpy(flippedAnims, newFlippedAnims, JJ2PANIMS * sizeof(Anim *));

	birds = flockSize;

	shield = JJ2S_NONE;

	reset(startX, startY);


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;


	// Fur colours

	start = offsets[player->cols[0]];
	length = lengths[player->cols[0]];

	for (count = 0; count < 16; count++)
		palette[count + 16].r = palette[count + 16].g = palette[count + 16].b =
			(count * length / 8) + start;


	// Bandana colours

	start = offsets[player->cols[1]];
	length = lengths[player->cols[1]];

	for (count = 0; count < 16; count++)
		palette[count + 24].r = palette[count + 24].g = palette[count + 24].b =
 			(count * length / 8) + start;


	// Gun colours

	start = offsets[player->cols[2]];
	length = lengths[player->cols[2]];

	for (count = 0; count < 9; count++)
		palette[count + 32].r = palette[count + 32].g = palette[count + 32].b =
			(count * length / 8) + start;


	// Wristband colours

	start = offsets[player->cols[3]];
	length = lengths[player->cols[3]];

	for (count = 0; count < 8; count++)
		palette[count + 40].r = palette[count + 40].g = palette[count + 40].b =
			(count * length / 8) + start;


	return;

}


/**
 * Delete the JJ2 level player.
 */
JJ2LevelPlayer::~JJ2LevelPlayer () {

	return;

}


/**
 * Reset the player's position, energy etc.
 *
 * @param startX New x-coordinate
 * @param startY New y-coordinate
 */
void JJ2LevelPlayer::reset (int startX, int startY) {

	mod = NULL;
	energy = 5;
	floating = false;
	facing = true;
	animType = JJ2PA_STAND;
	event = JJ2PE_NONE;
	reaction = JJ2PR_NONE;
	reactionTime = 0;
	jumpHeight = JJ2PYO_JUMP;
	throwY = TTOF(256);
	fastFeetTime = 0;
	stopTime = 0;
	dx = 0;
	dy = 0;
	x = TTOF(startX);
	y = TTOF(startY);
	gems[0] = gems[1] = gems[2] = gems[3] = 0;
	coins = 0;

	return;

}


/**
 * Add to the player's tally of gems of a certain colour.
 *
 * @param colour The colour of the gem
 */
void JJ2LevelPlayer::addGem (int colour) {

	gems[colour]++;

	return;

}


/**
 * Centre the player horizontally on the nearest tile.
 */
void JJ2LevelPlayer::centreX () {

	x = ((x + JJ2PXO_MID) & ~32767) + F16 - JJ2PXO_MID;

	return;

}


/**
 * Centre the player vertically on the nearest tile.
 */
void JJ2LevelPlayer::centreY () {

	y = ((y + JJ2PYO_MID) & ~32767) + F16 - JJ2PYO_MID;

	return;

}


/**
 * Determine the player's current animation.
 *
 * @return The current animation
 */
Anim* JJ2LevelPlayer::getAnim () {

	return (facing? anims: flippedAnims)[animType];

}


/**
 * Determine the player's current energy level.
 *
 * @return Energy level
 */
int JJ2LevelPlayer::getEnergy () {

	return energy;

}


/**
 * Determine the direction the player is facing.
 *
 * @return True if the player is facing right
 */
bool JJ2LevelPlayer::getFacing () {

	return facing;

}


/**
 * Determine the number of gems, of a certain colour, the player has collected.
 *
 * @param colour The colour of the gems
 *
 * @return Number of gems collected
 */
int JJ2LevelPlayer::getGems (int colour) {

	return gems[colour];

}


/**
 * Determine whether or not the player is being accompanied by a bird.
 *
 * @return Whether or not the player is being accompanied by a bird
 */
int JJ2LevelPlayer::countBirds () {

	return birds;

}


/**
 * Deal with bullet collisions.
 *
 * @param source Player that fired the bullet (NULL if an event)
 * @param ticks Time
 *
 * @return Whether or not the hit was successful
 */
bool JJ2LevelPlayer::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != JJ2PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == player->team)) return false;


	if (player->hit(source)) {

		energy--;

		//if (bird) bird->hit();

		playSound(S_OW);

	}

	if (energy) {

		reaction = JJ2PR_HURT;
		reactionTime = ticks + JJ2PRT_HURT;

		if (dx < 0) dx = JJ2PXS_RUN;
		else dx = -JJ2PXS_RUN;
		dy = JJ2PYS_JUMP;

	} else {

		kill(source, ticks);

	}

	return true;

}


/**
 * Kill the player.
 *
 * @param source Player responsible for the kill (NULL if due to an event or time)
 * @param ticks time
 */
void JJ2LevelPlayer::kill (Player *source, unsigned int ticks) {

	if (reaction != JJ2PR_NONE) return;

	if (player->kill(source)) {

		energy = 0;
		player->lives--;

		reaction = JJ2PR_KILLED;
		reactionTime = ticks + JJ2PRT_KILLED;

	}

	return;

}


/**
 * Determine whether or not the player is overlapping the given area.
 *
 * @param left The x-coordinate of the left of the area
 * @param top The y-coordinate of the top of the area
 * @param width The width of the area
 * @param height The height of the area
 *
 * @return Whether or not there is an overlap
 */
bool JJ2LevelPlayer::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + JJ2PXO_R >= left) && (x + JJ2PXO_L < left + width) &&
		(y >= top) && (y + JJ2PYO_TOP < top + height);

}


/**
 * Handle the player's reaction.
 *
 * @param ticks Time
 *
 * @return The reaction the player has just finished
 */
JJ2PlayerReaction JJ2LevelPlayer::reacted (unsigned int ticks) {

	JJ2PlayerReaction oldReaction;

	if ((reaction != JJ2PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = JJ2PR_NONE;

		return oldReaction;

	}

	return JJ2PR_NONE;

}


/**
 * Set the player's position.
 *
 * @param newX New x-coordinate
 * @param newY New y-coordinate
 */
void JJ2LevelPlayer::setPosition (fixed newX, fixed newY) {

	x = newX;
	y = newY;

	return;

}


/**
 * Set the player's speed.
 *
 * @param newDx New x-speed
 * @param newDy New y-speed
 */
void JJ2LevelPlayer::setSpeed (fixed newDx, fixed newDy) {

	dx = newDx;
	if (newDy) dy = newDy;

	return;

}


/**
 * Called when the player has touched an event.
 *
 * @param touched The event that was touched
 * @param ticks Time
 * @param msps Ticks per step
 *
 * @return Whether or not the event should be destroyed.
 */
bool JJ2LevelPlayer::touchEvent (JJ2Event* touched, unsigned int ticks, int msps) {

	(void)msps;

	unsigned char type;
	bool fullPickup = false;

	type = touched->getType();

	switch (type) {

		case 34: // Ammo

			player->addAmmo(2, 3);
			player->addScore(100);

			return true;

		case 36: // Ammo

			player->addAmmo(1, 3);
			player->addScore(100);

			return true;

		case 37: // Ammo

			player->addAmmo(0, 3);
			player->addScore(100);

			return true;

		case 38: // TNT

			player->addAmmo(3, 3);
			player->addScore(100);

			return true;

		case 44:

			coins++;
			player->addScore(500);

			return true;

		case 45:

			coins += 5;
			player->addScore(1000);

			return true;

		case 59: // Board

			floating = true;

			return true;

		case 60: // Frozen green spring

			throwY = y - TTOF(14);
			dx = 0;
			event = JJ2PE_SPRING;

			break;

		case 61: // Rapid fire

			player->fireSpeed++;

			return true;

		case 62: // Spring crate

			throwY = y - TTOF(18);
			dx = 0;
			event = JJ2PE_SPRING;

			break;

		case 63: // Red gem

			gems[0]++;
			player->addScore(100);

			return true;

		case 64: // Green gem

			gems[1]++;
			player->addScore(100);

			return true;

		case 65: // Blue gem

			gems[2]++;
			player->addScore(100);

			return true;

		case 66: // Purple gem

			gems[3]++;
			player->addScore(100);

			return true;

		case 72: // Carrot

			if ((energy < 5) || fullPickup) {

				if (energy < 5) energy++;

				player->addScore(200);

				return true;

			}

			break;

		case 73: // Full carrot

			if ((energy < 5) || fullPickup) {

				energy = 5;

				return true;

			}

			break;

		case 80: // 1-up

			player->addLife();

			return true;

		case 83: // Checkpoint

			player->setCheckpoint(FTOT(x + JJ2PXO_MID), FTOT(y + JJ2PYO_MID));

			break;

		case 85: // Red spring

			throwY = y - TTOF(8);
			dx = 0;
			event = JJ2PE_SPRING;

			break;

		case 86: // Green spring

			throwY = y - TTOF(14);
			dx = 0;
			event = JJ2PE_SPRING;

			break;

		case 87: // Blue spring

			throwY = y - TTOF(18);
			dx = 0;
			event = JJ2PE_SPRING;

			break;

		case 88: //Invincibility

			reaction = JJ2PR_INVINCIBLE;
			reactionTime = ticks + JJ2PRT_INVINCIBLE;

			return true;

		case 91: // Horizontal red spring

			if (true) throwX = x + TTOF(7);
			else throwX = x - TTOF(7);

			dy = 0;

			break;

		case 92: // Horizontal green spring

			if (true) throwX = x + TTOF(14);
			else throwX = x - TTOF(14);

			dy = 0;

			break;

		case 93: // Horizontal blue spring

			if (true) throwX = x + TTOF(18);
			else throwX = x - TTOF(18);

			dy = 0;

			break;

		case 96: // Helicarrot

			floating = true;

			return true;

		case 192: // Gem ring

			gems[0] += 8;

			return true;

		default:

			if (((type >= 141) && (type <= 147)) || ((type >= 154) && (type <= 182))) {

				// Food
				player->addScore(50);

				return true;

			}

			break;

	}

	return false;

}


/**
 * Fill a buffer with player data.
 *
 * @param buffer The buffer
 */
void JJ2LevelPlayer::send (unsigned char *buffer) {

	buffer[9] = birds;
	buffer[23] = energy;
	buffer[25] = shield;
	buffer[26] = floating;
	buffer[27] = getFacing();
	buffer[29] = jumpHeight >> 24;
	buffer[30] = (jumpHeight >> 16) & 255;
	buffer[31] = (jumpHeight >> 8) & 255;
	buffer[32] = jumpHeight & 255;
	buffer[33] = throwY >> 24;
	buffer[34] = (throwY >> 16) & 255;
	buffer[35] = (throwY >> 8) & 255;
	buffer[36] = throwY & 255;
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
void JJ2LevelPlayer::receive (unsigned char *buffer) {

	int count;

	switch (buffer[1]) {

		case MT_P_ANIMS:

			for (count = 0; count < JJ2PANIMS; count++) {

				anims[count] = jj2Level->getPlayerAnim(buffer[3], count, false);
				flippedAnims[count] = jj2Level->getPlayerAnim(buffer[3], count, true);

			}

			break;

		case MT_P_TEMP:

			birds = buffer[9];
			energy = buffer[23];
			shield = (JJ2Shield)buffer[25];
			floating = buffer[26];
			facing = buffer[27];
			jumpHeight = (buffer[29] << 24) + (buffer[30] << 16) + (buffer[31] << 8) + buffer[32];
			throwY = (buffer[33] << 24) + (buffer[34] << 16) + (buffer[35] << 8) + buffer[36];
			x = (buffer[37] << 24) + (buffer[38] << 16) + (buffer[39] << 8) + buffer[40];
			y = (buffer[41] << 24) + (buffer[42] << 16) + (buffer[43] << 8) + buffer[44];

			break;

	}

	return;

}

