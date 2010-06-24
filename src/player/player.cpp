
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
#include "bonusplayer.h"
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

	levelPlayer = new LevelPlayer(this);
	bonusPlayer = new BonusPlayer(this);

	bird = NULL;

	name = NULL;

	return;

}


Player::~Player () {

	deinit();

	delete levelPlayer;
	delete bonusPlayer;

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


void Player::reset () {

	int count;

	levelPlayer->reset();
	bonusPlayer->reset();

	if (bird) bird->reset();

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

	score += addedScore * 10;

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


void Player::send (unsigned char *buffer) {

	// Copy data to be sent to clients/server

	buffer[3] = pcontrols[C_UP];
	buffer[4] = pcontrols[C_DOWN];
	buffer[5] = pcontrols[C_LEFT];
	buffer[6] = pcontrols[C_RIGHT];
	buffer[7] = pcontrols[C_JUMP];
	buffer[8] = pcontrols[C_FIRE];
	buffer[9] = bird? 1: 0;
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

	levelPlayer->send(buffer);

	return;

}


void Player::receive (unsigned char *buffer) {

	// Interpret data received from client/server

	if (buffer[1] == MT_P_TEMP) {

		pcontrols[C_UP] = buffer[3];
		pcontrols[C_DOWN] = buffer[4];
		pcontrols[C_LEFT] = buffer[5];
		pcontrols[C_RIGHT] = buffer[6];
		pcontrols[C_JUMP] = buffer[7];
		pcontrols[C_SWIM] = buffer[45];
		pcontrols[C_FIRE] = buffer[8];
		pcontrols[C_CHANGE] = false;

		if ((buffer[9] & 1) && !bird)
			bird = new Bird(this, FTOT(levelPlayer->getX()), FTOT(levelPlayer->getY()));

		if (!(buffer[9] & 1) && bird) {

			delete bird;
			bird = NULL;

		}

		ammo[0] = (buffer[10] << 8) + buffer[11];
		ammo[1] = (buffer[12] << 8) + buffer[13];
		ammo[2] = (buffer[14] << 8) + buffer[15];
		ammo[3] = (buffer[16] << 8) + buffer[17];
		ammoType = buffer[18] - 1;
		score = (buffer[19] << 24) + (buffer[20] << 16) + (buffer[21] << 8) + buffer[22];
		lives = buffer[24];
		fireSpeed = buffer[28];

	}

	levelPlayer->receive(buffer);

	return;

}

