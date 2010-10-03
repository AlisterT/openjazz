
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
 * Copyright (c) 2005-2010 Alister Thomson
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


Player::Player () {

	levelPlayer = NULL;
	bonusPlayer = NULL;
	jj2LevelPlayer = NULL;

	name = NULL;

	return;

}


Player::~Player () {

	deinit();

	return;

}


void Player::init (char *playerName, unsigned char *playerCols, unsigned char newTeam) {

	// Clear existing player
	deinit();

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
	bird = false;
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


void Player::reset (unsigned char x, unsigned char y) {

	if (levelPlayer) levelPlayer->reset(x, y);
	else if (jj2LevelPlayer) jj2LevelPlayer->reset(x, y);

	return;

}


void Player::reset (LevelType levelType, Anim** anims, unsigned char x, unsigned char y) {

	int count;

	if (levelPlayer) {

		bird = levelPlayer->hasBird();
		delete levelPlayer;
		levelPlayer = NULL;

	}

	if (bonusPlayer) {

		delete bonusPlayer;
		bonusPlayer = NULL;

	}

	if (jj2LevelPlayer) {

		bird = jj2LevelPlayer->hasBird();
		delete jj2LevelPlayer;
		jj2LevelPlayer = NULL;

	}

	switch (levelType) {

		case LT_LEVEL:

			levelPlayer = new LevelPlayer(this, anims, x, y, bird);

			break;

		case LT_BONUS:

			bonusPlayer = new BonusPlayer(this, anims, x, y);

			break;

		case LT_JJ2LEVEL:

			jj2LevelPlayer = new JJ2LevelPlayer(this, anims, x, y, bird);

			break;

	}

	for (count = 0; count < PCONTROLS; count++) pcontrols[count] = false;

	return;

}


BonusPlayer* Player::getBonusPlayer () {

	return bonusPlayer;

}


unsigned char * Player::getCols () {

	return cols;

}


char * Player::getName () {

	return name;

}


JJ2LevelPlayer* Player::getJJ2LevelPlayer () {

	return jj2LevelPlayer;

}


LevelPlayer* Player::getLevelPlayer () {

	return levelPlayer;

}


void Player::setControl (int control, bool state) {

	pcontrols[control] = state;

	return;

}


bool Player::getControl (int control) {

	return pcontrols[control];

}


void Player::addScore (int addedScore) {

	score += addedScore;

	return;

}

int Player::getScore () {

	return score;

}


void Player::addLife () {

	if (lives < 99) lives++;

	return;

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


unsigned char Player::getTeam () {

	return team;

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
	if (!levelPlayer) buffer[9] = bird? 1: 0;
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
		if (!levelPlayer) bird = buffer[9] & 1;
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

