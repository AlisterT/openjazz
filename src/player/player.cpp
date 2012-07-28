
/**
 *
 * @file player.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 3rd February 2009: Created player.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 19th March 2009: Created sprite.cpp from parts of event.cpp and player.cpp
 * 18th July 2009: Created playerframe.cpp from parts of player.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the creation and destruction of players, and their interactions
 * with other objects.
 *
 */


#include "bird.h"
#include "bonusplayer.h"
#include "jj2levelplayer.h"
#include "levelplayer.h"

#include "baselevel.h"
#include "game/game.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/video.h"
#include "util.h"

#include <string.h>


/**
 * Create the player.
 */
Player::Player () {

	levelPlayer = NULL;
	bonusPlayer = NULL;
	jj2LevelPlayer = NULL;

	name = NULL;

	return;

}


/**
 * Delete the player.
 */
Player::~Player () {

	deinit();

	return;

}


/**
 * Initialise player data.
 *
 * Deinitialise any existing player data, assign properties and initial values.
 *
 * @param playerName Name (displayed in multiplayer games)
 * @param playerCols Colours (only used in multiplayer games)
 * @param newTeam Team (in multiplayer games)
 */
void Player::init (Game* owner, char *playerName, unsigned char *playerCols, unsigned char newTeam) {

	// Clear existing player
	deinit();

	// Assign owner
	game = owner;

	// Assign name
	name = createString(playerName);

	// Assign initial values
	score = 0;
	lives = 3;
	ammoType = -1;
	ammo[0] = 0;
	ammo[1] = 0;
	ammo[2] = 0;
	ammo[3] = 0;
	fireSpeed = 0;
	flockSize = 0;
	team = newTeam;
	teamScore = 0;

	if (playerCols) {

		memcpy(cols, playerCols, PCOLOURS);

	} else {

		cols[0] = CHAR_FUR;
		cols[1] = CHAR_BAND;
		cols[2] = CHAR_GUN;
		cols[3] = CHAR_WBAND;

	}

	return;

}


/**
 * Deinitialise player data.
 */
void Player::deinit () {

	if (levelPlayer) delete levelPlayer;
	if (bonusPlayer) delete bonusPlayer;
	if (jj2LevelPlayer) delete levelPlayer;
	levelPlayer = NULL;
	bonusPlayer = NULL;
	jj2LevelPlayer = NULL;

	if (name) delete[] name;
	name = NULL;

	return;

}


/**
 * Reset the player's current level player.
 *
 * @param x The level player's new grid x-coordinate
 * @param y The level player's new grid y-coordinate
 */
void Player::reset (unsigned char x, unsigned char y) {

	if (levelPlayer) levelPlayer->reset(x, y);
	else if (jj2LevelPlayer) jj2LevelPlayer->reset(x, y);

	return;

}


/**
 * Create a new level player for the player (and delete any existing one).
 *
 * @param levelType The type of level for which to create a level player
 * @param anims New level player animations
 * @param x The level player's new grid x-coordinate
 * @param y The level player's new grid y-coordinate
 */
void Player::reset (LevelType levelType, Anim** anims, unsigned char x, unsigned char y) {

	int count;

	if (levelPlayer) {

		flockSize = levelPlayer->countBirds();
		delete levelPlayer;
		levelPlayer = NULL;

	}

	if (bonusPlayer) {

		delete bonusPlayer;
		bonusPlayer = NULL;

	}

	if (jj2LevelPlayer) {

		flockSize = jj2LevelPlayer->countBirds();
		delete jj2LevelPlayer;
		jj2LevelPlayer = NULL;

	}

	switch (levelType) {

		case LT_LEVEL:

			levelPlayer = new LevelPlayer(this, anims, x, y, flockSize);

			break;

		case LT_BONUS:

			bonusPlayer = new BonusPlayer(this, anims, x, y);

			break;

		case LT_JJ2LEVEL:

			jj2LevelPlayer = new JJ2LevelPlayer(this, anims, x, y, flockSize);

			break;

	}

	for (count = 0; count < PCONTROLS; count++) pcontrols[count] = false;

	return;

}


/**
 * Get the player's JJ1 bonus level player.
 *
 * @return The JJ1 bonus level player
 */
BonusPlayer* Player::getBonusPlayer () {

	return bonusPlayer;

}


/**
 * Get the player's colours.
 *
 * @return The player's colours
 */
unsigned char * Player::getCols () {

	return cols;

}


/**
 * Get the player's name.
 *
 * @return The player's name
 */
char * Player::getName () {

	return name;

}


/**
 * Get the player's JJ2 level player.
 *
 * @return The JJ2 level player
 */
JJ2LevelPlayer* Player::getJJ2LevelPlayer () {

	return jj2LevelPlayer;

}


/**
 * Get the player's JJ1 level player.
 *
 * @return The JJ1 level player
 */
LevelPlayer* Player::getLevelPlayer () {

	return levelPlayer;

}


/**
 * Set the state of the specified control.
 *
 * @param control Affected control
 * @param state New state
 */
void Player::setControl (int control, bool state) {

	pcontrols[control] = state;

	return;

}


/**
 * Get the state of the specified control.
 *
 * @param control The control
 *
 * @return State
 */
bool Player::getControl (int control) {

	return pcontrols[control];

}


/**
 * Add to the player's total score.
 *
 * @param addedScore The amount to be added
 */
void Player::addScore (int addedScore) {

	score += addedScore;

	return;

}


/**
 * Get the player's score.
 */
int Player::getScore () {

	return score;

}


/**
 * Add an extra life to the player.
 */
void Player::addLife () {

	if (lives < 99) lives++;

	return;

}


/**
 * Get the number of extra lives.
 *
 * @return Number of extra lives
 */
int Player::getLives () {

	return lives;

}


/**
 * Add ammo to the player's arsenal.
 *
 * @param type Type of ammo
 * @param amount Amount of ammo to add
 */
void Player::addAmmo (int type, int amount) {

	if (!ammo[type]) ammoType = type;
	ammo[type] += amount;

	return;

}


/**
 * Get the type of ammo or the total amount of ammo of the current type.
 *
 * @param amount False for type, true for amount
 *
 * @param The requested value
 */
int Player::getAmmo (bool amount) {

	return amount? ammo[ammoType]: ammoType;

}


/**
 * Get the player's team
 *
 * @return Team number
 */
unsigned char Player::getTeam () {

	return team;

}


/**
 * Deal with bullet collisions.
 *
 * @param source Player that fired the bullet (NULL if an event)
 *
 * @return Whether or not the hit was successful
 */
bool Player::hit (Player *source) {

	return game->getMode()->hit(source, this);

}


/**
 * Kill the player.
 *
 * @param source Player responsible for the kill (NULL if due to an event or time)
 *
 * @return Whether or not the kill was successful
 */
bool Player::kill (Player *source) {

	return game->getMode()->kill(game, source, this);

}


/**
 * Set the checkpoint
 *
 * @param gridX X-coordinate (in tiles) of the checkpoint
 * @param gridY Y-coordinate (in tiles) of the checkpoint
 */
void Player::setCheckpoint (unsigned char gridX, unsigned char gridY) {

	game->setCheckpoint(gridX, gridY);

	return;

}


/**
 * Outcome of level being completed
 *
 * @param gridX X-coordinate (in tiles) of finishing position
 * @param gridY Y-coordinate (in tiles) of finishing position
 *
 * @return Whether or not the end-of-level signpost should be destroyed
 */
bool Player::endOfLevel (unsigned char gridX, unsigned char gridY) {

	return game->getMode()->endOfLevel(game, this, gridX, gridY);

}


/**
 * Copy data to be sent to clients/server
 */
void Player::send (unsigned char *buffer) {

	buffer[3] = pcontrols[C_UP];
	buffer[4] = pcontrols[C_DOWN];
	buffer[5] = pcontrols[C_LEFT];
	buffer[6] = pcontrols[C_RIGHT];
	buffer[7] = pcontrols[C_JUMP];
	buffer[8] = pcontrols[C_FIRE];
	if (!levelPlayer) buffer[9] = flockSize;
	buffer[10] = ammo[0] >> 8;
	buffer[11] = ammo[0] & 255;
	buffer[12] = ammo[1] >> 8;
	buffer[13] = ammo[1] & 255;
	buffer[14] = ammo[2] >> 8;
	buffer[15] = ammo[2] & 255;
	buffer[16] = ammo[3] >> 8;
	buffer[17] = ammo[3] & 255;
	buffer[18] = ammoType + 1;
	buffer[19] = score >> 24;
	buffer[20] = (score >> 16) & 255;
	buffer[21] = (score >> 8) & 255;
	buffer[22] = score & 255;
	buffer[24] = lives;
	buffer[28] = fireSpeed;
	buffer[45] = pcontrols[C_SWIM];

	if (levelPlayer) levelPlayer->send(buffer);
	if (jj2LevelPlayer) jj2LevelPlayer->send(buffer);

	return;

}


/**
 * Interpret data received from client/server
 */
void Player::receive (unsigned char *buffer) {

	if (buffer[1] == MT_P_TEMP) {

		pcontrols[C_UP] = buffer[3];
		pcontrols[C_DOWN] = buffer[4];
		pcontrols[C_LEFT] = buffer[5];
		pcontrols[C_RIGHT] = buffer[6];
		pcontrols[C_JUMP] = buffer[7];
		pcontrols[C_SWIM] = buffer[45];
		pcontrols[C_FIRE] = buffer[8];
		pcontrols[C_CHANGE] = false;
		if (!levelPlayer) flockSize = buffer[9];
		ammo[0] = (buffer[10] << 8) + buffer[11];
		ammo[1] = (buffer[12] << 8) + buffer[13];
		ammo[2] = (buffer[14] << 8) + buffer[15];
		ammo[3] = (buffer[16] << 8) + buffer[17];
		ammoType = buffer[18] - 1;
		score = (buffer[19] << 24) + (buffer[20] << 16) + (buffer[21] << 8) + buffer[22];
		lives = buffer[24];
		fireSpeed = buffer[28];

	}

	if (levelPlayer) levelPlayer->receive(buffer);
	if (jj2LevelPlayer) jj2LevelPlayer->receive(buffer);

	return;

}

