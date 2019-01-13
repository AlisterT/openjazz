
/**
 *
 * @file jj1levelplayer.cpp
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
 * - 1st August 2012: Renamed levelplayer.cpp to jj1levelplayer.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the creation and destruction of players in levels, and their
 * interactions with other level objects.
 *
 */


#include "../jj1event/jj1event.h"
#include "../jj1level.h"
#include "jj1bird.h"
#include "jj1levelplayer.h"

#include "game/game.h"
#include "io/sound.h"
#include "setup.h"

#include <string.h>


/**
 * Create a JJ1 level player.
 *
 * @param parent The game player corresponding to this level player.
 * @param newAnims Animations
 * @param startX Starting position x-coordinate
 * @param startY Starting position y-coordinate
 * @param flockSize The number of birds accompanying the player
 */
JJ1LevelPlayer::JJ1LevelPlayer (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY, int flockSize) {

	int offsets[15] = {PCO_GREY, PCO_SGREEN, PCO_BLUE, PCO_RED, PCO_LGREEN,
		PCO_LEVEL1, PCO_YELLOW, PCO_LEVEL2, PCO_ORANGE, PCO_LEVEL3, PCO_LEVEL4,
		PCO_SANIM, PCO_LANIM, PCO_LEVEL5, 256};
	int count, start, length;


	player = parent;

	memcpy(anims, newAnims, JJ1PANIMS * sizeof(Anim *));

	birds = NULL;

	for (count = 0; count < flockSize; count++)
		birds = new JJ1Bird(birds, this, startX, startY - 2);

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


/**
 * Delete the JJ1 level player.
 */
JJ1LevelPlayer::~JJ1LevelPlayer () {

	if (birds) delete birds;

	return;

}


/**
 * Reset the player's position, energy etc.
 *
 * @param startX New x-coordinate
 * @param startY New y-coordinate
 */
void JJ1LevelPlayer::reset (int startX, int startY) {

	x = TTOF(startX);
	y = TTOF(startY);
	dx = 0;
	dy = 0;

	eventType = JJ1PE_NONE;
	energy = 4;
	flying = false;
	facing = true;
	udx = 0;
	animType = PA_RSTAND;
	reaction = PR_NONE;
	reactionTime = 0;
	jumpHeight = PYO_JUMP;
	targetY = TTOF(LH);
	fastFeetTime = 0;
	warpTime = 0;
	fireTime = 0;
	fireAnimTime = 0;

	return;

}


/**
 * Add to the player's item tally.
 */
void JJ1LevelPlayer::addItem () {

	items++;

	return;

}


/**
 * If the player is tied to the event from the given tile, untie it.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 */
void JJ1LevelPlayer::clearEvent (unsigned char gridX, unsigned char gridY) {

	// If the location matches, clear the event

	if ((gridX == eventX) && (gridY == eventY)) eventType = JJ1PE_NONE;

	return;

}


/**
 * Determine the player's current animation.
 *
 * @return The current animation
 */
Anim* JJ1LevelPlayer::getAnim () {

	return anims[animType];

}


/**
 * Determine the number of enemies the player has killed.
 *
 * @return Number of enemies killed
 */
int JJ1LevelPlayer::getEnemies () {

	return enemies;

}


/**
 * Determine the player's current energy level.
 *
 * @return Energy level
 */
int JJ1LevelPlayer::getEnergy () {

	return energy;

}


/**
 * Determine the direction the player is facing.
 *
 * @return True if the player is facing right
 */
bool JJ1LevelPlayer::getFacing () {

	return facing;

}


/**
 * Get the horizontal speed of the player.
 *
 * @return The player's horizontal speed
 */
fixed JJ1LevelPlayer::getXSpeed () {

	return dx;

}


/**
 * Determine the number of items the player has collected.
 *
 * @return Number of items collected
 */
int JJ1LevelPlayer::getItems () {

	return items;

}


/**
 * Determine whether or not the player is being accompanied by a bird.
 *
 * @return Whether or not the player is being accompanied by a bird
 */
int JJ1LevelPlayer::countBirds () {

	if (birds) return birds->getFlockSize();

	return 0;

}


/**
 * Determine whether or not the player has collected a gem.
 *
 * @return Whether or not the player has collected a gem
 */
bool JJ1LevelPlayer::hasGem () {

	return gem;

}


/**
 * Deal with bullet collisions.
 *
 * @param source Player that fired the bullet (NULL if an event)
 * @param ticks Time
 *
 * @return Whether or not the hit was successful
 */
bool JJ1LevelPlayer::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == player->team)) return false;

	// Hits to the shield only affect the shield
	if (shield) {

		if (shield == 2) shield = 0;
		else shield--;

		reaction = PR_SHIELDED;
		reactionTime = ticks + PRT_SHIELDED;

		return true;

	}

	// Hits only cause damage in applicable game modes
	if (player->hit(source)) {

		// Hits don't cause damage with a bird, but do scare the bird away
		if (birds) birds->hit();
		else energy--;

	}

	playSound(S_UPLOOP);

	if (energy) {

		reaction = PR_HURT;
		reactionTime = ticks + PRT_HURT;

		if (dx < 0) dx = PXS_RUN;
		else dx = -PXS_RUN;
		dy = PYS_JUMP;

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
void JJ1LevelPlayer::kill (Player *source, unsigned int ticks) {

	if (reaction != PR_NONE) return;

	if (player->kill(source)) {

		energy = 0;
		player->lives--;

		reaction = PR_KILLED;
		reactionTime = ticks + PRT_KILLED;

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
bool JJ1LevelPlayer::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + PXO_R >= left) && (x + PXO_L < left + width) &&
		(y >= top) && (y + PYO_TOP < top + height);

}


/**
 * Handle the player's reaction.
 *
 * @param ticks Time
 *
 * @return The reaction the player has just finished
 */
JJ1PlayerReaction JJ1LevelPlayer::reacted (unsigned int ticks) {

	JJ1PlayerReaction oldReaction;

	if ((reaction != PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = PR_NONE;

		return oldReaction;

	}

	return PR_NONE;

}


/**
 * Tie the player to the platform from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param shiftX Change in horizontal position
 * @param newY New vertical position
 */
void JJ1LevelPlayer::setPlatform (unsigned char gridX, unsigned char gridY, fixed shiftX, fixed newY) {

	eventType = JJ1PE_PLATFORM;
	eventX = gridX;
	eventY = gridY;

	if (((shiftX < 0) && !level->checkMaskUp(x + PXO_L + shiftX, y + PYO_MID)) ||
		((shiftX > 0) && !level->checkMaskUp(x + PXO_R + shiftX, y + PYO_MID))) {

		x += shiftX;

	}

	y = newY;

	return;

}


/**
 * Take the event from the given tile.
 *
 * @param event Event type
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param ticks Time
 *
 * @return Whether or not the event should be destroyed.
 */
bool JJ1LevelPlayer::takeEvent (JJ1EventType* event, unsigned char gridX, unsigned char gridY, unsigned int ticks) {

	switch (event->modifier) {

		case 41: // Bonus level

			if (energy) level->setNext(event->multiA, event->multiB);

			// FALLTHROUGH

		case 8: // Boss
		case 27: // End of level

			if (!energy) return false;

			if (!player->endOfLevel(gridX, gridY)) return false;

			level->setStage(LS_END);

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

			if ((energy == 4) && setup.leaveUnneeded) return false;

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

			level->addTimer(2 * 60);

			break;

		case 10: // Checkpoint

			player->setCheckpoint(gridX, gridY);

			break;

		case 11: // Item

			break;

		case 12: // Rapid fire

			player->fireSpeed++;
			fireTime = 0;

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

			// Sky blue flash
			level->flash(135, 206, 235, 320);

			// Speed up music

			setMusicTempo(MUSIC_FAST);

			break;

		case 30: // TNT

			player->addAmmo(4, 1);

			break;

		case 31: // Water level

			level->setWaterLevel(gridY + 1);

			break;

		case 33: // 1-hit shield

			if ((shield >= 1) && setup.leaveUnneeded) return false;
			else shield = 1;

			break;

		case 34: // Bird

			if (birds && !setup.manyBirds) return false;

			birds = new JJ1Bird(birds, this, gridX, gridY);

			break;

		case 35: // Airboard, etc.

			if (flying && setup.leaveUnneeded) return false;

			flying = true;

			break;

		case 36: // 4-hit shield

			if ((shield == 5) && setup.leaveUnneeded) return false;

			shield = 5;

			break;

		case 37: // Diamond

			if (gem && setup.leaveUnneeded) return false;

			gem = true;

			// Yellow flash
			level->flash(255, 255, 0, 320);

			break;

		case 39: // Ammo

			player->addAmmo(3, 15);

			break;

		case 40: // Ammo

			player->addAmmo(3, 2);

			break;

		default:

			return false;

	}

	player->addScore(event->points * 10);

	// Add to player's enemy/item tally
	// If the event hurts and can be killed, it is an enemy
	// Anything else that scores is an item
	if ((event->modifier == 0) && event->strength) enemies++;
	else if (event->points) items++;

	return true;

}


/**
 * Called when the player has touched the event from the given tile.
 *
 * @param event Event type
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param ticks Time
 *
 * @return Whether or not the event should be destroyed.
 */
bool JJ1LevelPlayer::touchEvent (JJ1EventType* event, unsigned char gridX, unsigned char gridY, unsigned int ticks) {

	if ((event->movement == 37) || (event->movement == 38)) {

		// Repel

		if (event->multiB) {

			if (event->multiA > 0) {

				udx = (event->magnitude < 0 ? -ITOF(240): ITOF(240));

				eventType = JJ1PE_REPELUP;
				targetY = TTOF(gridY) - ITOF(event->multiA * 3);

			} else {

				eventType = JJ1PE_REPELDOWN;

			}

			eventX = gridX;
			eventY = gridY;

			dy = event->multiA * -F24;

		} else {

			eventType = JJ1PE_REPELH;
			eventX = gridX;
			eventY = gridY;

		}

	}

	switch (event->modifier) {

		case 0: // Hurt
		case 8: // Boss

			if ((event->movement < 37) || (event->movement > 44)) {

				if (reaction != PR_SHIELDED) hit(NULL, ticks);

				if (reaction == PR_SHIELDED) {

					level->hitEvent(gridX, gridY, 255, this, ticks);

					return true;

				}

			}

			break;

		case 7: // Used with destructible blocks, but should not destroy on contact

			break;

		case 13: // Warp

			if (!warpTime) {

				warpX = event->multiA;
				warpY = event->multiB;
				warpTime = ticks + T_WARP;

				// White flash
				level->flash(255, 255, 255, T_WARP);

			}

			break;

		case 28: // Belt

			if (event->magnitude < 0) {

				if (!level->checkMaskDown(x + PXO_L + (event->magnitude * 64), y + PYO_MID))
					x += event->magnitude * 64;

			} else {

				if (!level->checkMaskDown(x + PXO_R + (event->magnitude * 64), y + PYO_MID))
					x += event->magnitude * 64;

			}

			break;

		case 29: // Upwards spring

			eventType = JJ1PE_SPRING;
			eventX = gridX;
			eventY = gridY;
			targetY = TTOF(gridY) + (event->magnitude * ITOF(21));

			playSound(event->sound);

			break;

		case 31: // Water level

			level->setWaterLevel(gridY + 1);

			break;

		case 32: // Float up / sideways

			if (event->multiB) {

				eventType = JJ1PE_FLOAT;
				eventX = gridX;
				eventY = gridY;
				targetY = TTOF(gridY) - (event->multiA * ITOF(17));

			} else {

				eventType = JJ1PE_FLOATH;
				eventX = gridX;
				eventY = gridY;

			}

			break;

		case 38: // Airboard, etc. off

			flying = false;

			break;

		default:

			if (!event->strength) return takeEvent(event, gridX, gridY, ticks);

			break;

	}

	return false;

}


/**
 * Fill a buffer with player data.
 *
 * @param buffer The buffer
 */
void JJ1LevelPlayer::send (unsigned char *buffer) {

	// Copy data to be sent to clients/server

	buffer[9] = countBirds();
	buffer[23] = energy;
	buffer[25] = shield;
	buffer[26] = flying;
	buffer[27] = getFacing();
	buffer[29] = jumpHeight >> 24;
	buffer[30] = (jumpHeight >> 16) & 255;
	buffer[31] = (jumpHeight >> 8) & 255;
	buffer[32] = jumpHeight & 255;
	buffer[33] = targetY >> 24;
	buffer[34] = (targetY >> 16) & 255;
	buffer[35] = (targetY >> 8) & 255;
	buffer[36] = targetY & 255;
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
void JJ1LevelPlayer::receive (unsigned char *buffer) {

	int count;

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_P_ANIMS:

			for (count = 0; count < JJ1PANIMS; count++)
				anims[count] = level->getAnim(buffer[MTL_P_ANIMS + count]);

			break;

		case MT_P_TEMP:

			if ((buffer[9] > 0) && (birds == NULL)) {

				birds = new JJ1Bird(birds, this, FTOT(x), FTOT(y) - 2);

			}

			if (birds) {

				birds = birds->setFlockSize(buffer[9]);

			}

			energy = buffer[23];
			shield = buffer[25];
			flying = buffer[26];
			facing = buffer[27];
			jumpHeight = (buffer[29] << 24) + (buffer[30] << 16) + (buffer[31] << 8) + buffer[32];
			targetY = (buffer[33] << 24) + (buffer[34] << 16) + (buffer[35] << 8) + buffer[36];
			x = (buffer[37] << 24) + (buffer[38] << 16) + (buffer[39] << 8) + buffer[40];
			y = (buffer[41] << 24) + (buffer[42] << 16) + (buffer[43] << 8) + buffer[44];

			break;

	}

	return;

}

