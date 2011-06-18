
/**
 *
 * @file levelplayer.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 24th June 2010: Created levelplayer.cpp from parts of player.cpp
 * 29th June 2010: Created jj2levelplayer.cpp from parts of levelplayer.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2011 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
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


/**
 * Create a JJ1 level player.
 *
 * @param parent The game player corresponding to this level player.
 * @param newAnims Animations
 * @param startX Starting position x-coordinate
 * @param startY Starting position y-coordinate
 * @param flockSize The number of birds accompanying the player
 */
LevelPlayer::LevelPlayer (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY, int flockSize) {

	int offsets[15] = {PCO_GREY, PCO_SGREEN, PCO_BLUE, PCO_RED, PCO_LGREEN,
		PCO_LEVEL1, PCO_YELLOW, PCO_LEVEL2, PCO_ORANGE, PCO_LEVEL3, PCO_LEVEL4,
		PCO_SANIM, PCO_LANIM, PCO_LEVEL5, 256};
	int count, start, length;


	player = parent;

	memcpy(anims, newAnims, PANIMS * sizeof(Anim*));

	birds = NULL;

	for (count = 0; count < flockSize; count++)
		birds = new Bird(birds, this, startX, startY - 2);

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
LevelPlayer::~LevelPlayer () {

	if (birds) delete birds;

	return;

}


/**
 * Reset the player's position, energy etc.
 *
 * @param startX New x-coordinate
 * @param startY New y-coordinate
 */
void LevelPlayer::reset (unsigned char startX, unsigned char startY) {

	event = LPE_NONE;
	energy = 4;
	floating = false;
	facing = true;
	animType = PA_RSTAND;
	reaction = PR_NONE;
	reactionTime = 0;
	jumpHeight = PYO_JUMP;
	jumpY = TTOF(LH);
	fastFeetTime = 0;
	warpTime = 0;
	dx = 0;
	dy = 0;
	x = TTOF(startX);
	y = TTOF(startY);

	return;

}


/**
 * Add to the player's item tally.
 */
void LevelPlayer::addItem () {

	items++;

	return;

}


/**
 * If the player is tied to the event from the given tile, untie it.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 */
void LevelPlayer::clearEvent (unsigned char gridX, unsigned char gridY) {

	// If the location matches, clear the event

	if ((gridX == eventX) && (gridY == eventY)) event = LPE_NONE;

	return;

}


/**
 * Determine the player's current animation.
 *
 * @return The current animation
 */
Anim* LevelPlayer::getAnim () {

	return anims[animType];

}


/**
 * Determine the number of enemies the player has killed.
 *
 * @return Number of enemies killed
 */
int LevelPlayer::getEnemies () {

	return enemies;

}


/**
 * Determine the player's current energy level.
 *
 * @return Energy level
 */
int LevelPlayer::getEnergy () {

	return energy;

}


/**
 * Determine the direction the player is facing.
 *
 * @return True if the player is facing right
 */
bool LevelPlayer::getFacing () {

	return facing;

}


/**
 * Determine the number of items the player has collected.
 *
 * @return Number of items collected
 */
int LevelPlayer::getItems () {

	return items;

}


/**
 * Determine whether or not the player is being accompanied by a bird.
 *
 * @return Whether or not the player is being accompanied by a bird
 */
int LevelPlayer::countBirds () {

	if (birds) return birds->getFlockSize();

	return 0;

}


/**
 * Determine whether or not the player has collected a gem.
 *
 * @return Whether or not the player has collected a gem
 */
bool LevelPlayer::hasGem () {

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
bool LevelPlayer::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == player->team)) return false;


	if (shield == 3) shield = 0;
	else if (shield) shield--;
	else if (game->getMode()->hit(source, player)) {

		energy--;

		if (birds) birds->hit();

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


/**
 * Kill the player.
 *
 * @param source Player responsible for the kill (NULL if due to an event or time)
 * @param ticks time
 */
void LevelPlayer::kill (Player *source, unsigned int ticks) {

	if (reaction != PR_NONE) return;

	if (game->getMode()->kill(source, player)) {

		energy = 0;
		player->lives--;

		reaction = PR_KILLED;
		reactionTime = ticks + PRT_KILLED;

		if (game->getMode()->getMode() == M_SINGLE) level->flash(0, 0, 0, T_END << 1);

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
bool LevelPlayer::overlap (fixed left, fixed top, fixed width, fixed height) {

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
PlayerReaction LevelPlayer::reacted (unsigned int ticks) {

	PlayerReaction oldReaction;

	if ((reaction != PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = PR_NONE;

		return oldReaction;

	}

	return PR_NONE;

}


/**
 * Tie the player to the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 */
void LevelPlayer::setEvent (unsigned char gridX, unsigned char gridY) {

	EventType *set;

	set = level->getEvent(gridX, gridY);

	if (set->modifier == 29) {

		// Upwards spring
		jumpY = y + (set->magnitude * (F20 + F1));
		event = LPE_SPRING;

	} else if (set->modifier == 6) event = LPE_PLATFORM;
	else if (set->movement == 28) event = LPE_PLATFORM;
	else return;

	eventX = gridX;
	eventY = gridY;

	return;

}


/**
 * Set the player's position.
 *
 * @param newX New x-coordinate
 * @param newY New y-coordinate
 */
void LevelPlayer::setPosition (fixed newX, fixed newY) {

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
void LevelPlayer::setSpeed (fixed newDx, fixed newDy) {

	dx = newDx;
	if (newDy) dy = newDy;

	return;

}


/**
 * Take the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param ticks Time
 *
 * @return Whether or not the event should be destroyed.
 */
bool LevelPlayer::takeEvent (unsigned char gridX, unsigned char gridY, unsigned int ticks) {

	EventType* set;

	set = level->getEvent(gridX, gridY);

	switch (set->modifier) {

		case 41: // Bonus level

			if (energy) level->setNext(set->multiA, set->multiB);

			// The lack of a break statement is intentional

		case 8: // Boss
		case 27: // End of level

			if (!energy) return false;

			if (!game->getMode()->endOfLevel(player, gridX, gridY)) return false;

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

			if ((shield >= 2) && setup.leaveUnneeded) return false;

			if (shield < 2) shield = 2;

			break;

		case 34: // Bird

			if (birds && !setup.manyBirds) return false;

			birds = new Bird(birds, this, gridX, gridY);

			break;

		case 35: // Airboard, etc.

			if (floating && setup.leaveUnneeded) return false;

			floating = true;

			break;

		case 36: // 4-hit shield

			if ((shield == 6) && setup.leaveUnneeded) return false;

			shield = 6;

			break;

		case 37: // Diamond

			if (gem && setup.leaveUnneeded) return false;

			gem = true;

			// Yellow flash
			level->flash(255, 255, 0, 320);

			break;

		default:

			return false;

	}

	player->addScore(set->points * 10);

	// Add to player's enemy/item tally
	// If the event hurts and can be killed, it is an enemy
	// Anything else that scores is an item
	if ((set->modifier == 0) && set->strength) enemies++;
	else if (set->points) items++;

	return true;

}


/**
 * Called when the player has touched the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param ticks Time
 * @param msps Ticks per step
 *
 * @return Whether or not the event should be destroyed.
 */
bool LevelPlayer::touchEvent (unsigned char gridX, unsigned char gridY, unsigned int ticks, int msps) {

	EventType* set;

	set = level->getEvent(gridX, gridY);

	switch (set->modifier) {

		case 0: // Hurt
		case 8: // Boss

			if ((set->movement < 37) || (set->movement > 44)) hit(NULL, ticks);

			break;

		case 7: // Used with destructible blocks, but should not destroy on contact

			break;

		case 13: // Warp

			if (!warpTime) {

				warpX = set->multiA;
				warpY = set->multiB;
				warpTime = ticks + T_WARP;

				// White flash
				level->flash(255, 255, 255, T_WARP);

			}

			break;

		case 28: // Belt

			x += set->magnitude * 4 * msps;

			break;

		case 29: // Upwards spring

			setEvent(gridX, gridY);

			level->playSound(set->sound);

			break;

		case 31: // Water level

			level->setWaterLevel(gridY);

			break;

		case 32: // Float up / sideways

			if (set->multiB) {

				eventX = gridX;
				eventY = gridY;
				event = LPE_FLOAT;

				if (dy > set->multiA * -F20)
					dy -= set->multiA * 320 * msps;

				jumpY = y - (8 * F16);

			} else if (set->magnitude < 0) {

				if (!level->checkMaskDown(x + PXO_L + (set->magnitude * 20 * msps), y + PYO_MID))
					x += set->magnitude * 20 * msps;

			} else {

				if (!level->checkMaskDown(x + PXO_R + (set->magnitude * 20 * msps), y + PYO_MID))
					x += set->magnitude * 20 * msps;

			}

			break;

		case 38: // Airboard, etc. off

			floating = false;

			break;

		default:

			if (!set->strength) return takeEvent(gridX, gridY, ticks);

			break;

	}

	return false;

}


/**
 * Fill a buffer with player data.
 *
 * @param buffer The buffer
 */
void LevelPlayer::send (unsigned char *buffer) {

	// Copy data to be sent to clients/server

	buffer[9] = countBirds();
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


/**
 * Adjust player data based on the contents of a given buffer.
 *
 * @param buffer The buffer
 */
void LevelPlayer::receive (unsigned char *buffer) {

	int count;

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_P_ANIMS:

			for (count = 0; count < PANIMS; count++)
				anims[count] = level->getAnim(buffer[MTL_P_ANIMS + count]);

			break;

		case MT_P_TEMP:

			if ((buffer[9] > 0) && (birds == NULL)) {

				birds = new Bird(birds, this, FTOT(x), FTOT(y) - 2);

			}

			if (birds) {

				birds = birds->setFlockSize(buffer[9]);

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

