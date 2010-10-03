
/**
 *
 * @file localgame.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 3rd October 2010: Created localgame.cpp from parts of game.cpp
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
 */


#include "game.h"
#include "gamemode.h"

#include "player/player.h"
#include "util.h"


/**
 * Create a single-player local game
 *
 * @param firstLevel File name of the first level to play
 * @param gameDifficulty Difficulty setting
 */
LocalGame::LocalGame (char *firstLevel, int gameDifficulty) {

	levelFile = createString(firstLevel);

	difficulty = gameDifficulty;

	mode = new SingleGameMode();

	// Create the player
	nPlayers = 1;
	localPlayer = players = new Player[1];
	localPlayer->init(characterName, NULL, 0);

	return;

}


/**
 * Destroy local game
 */
LocalGame::~LocalGame () {

	delete mode;

	return;

}


/**
 * Set the next level
 *
 * @param fileName The file name of the next level
 *
 * @return Error code
 */
int LocalGame::setLevel (char *fileName) {

	if (levelFile) delete[] levelFile;

	if (fileName) levelFile = createString(fileName);
	else levelFile = NULL;

	return E_NONE;

}


/**
 * No data is sent in local games
 *
 * @param buffer Data that will not be sent. First byte indicates length.
 */
void LocalGame::send (unsigned char *buffer) {

	// Do nothing

	return;

}


/**
 * Game iteration - nothing to be done in local games
 *
 * @param ticks Current time
 *
 * @return Error code
 */
int LocalGame::step (unsigned int ticks) {

	// Do nothing

	return E_NONE;

}


/**
 * No points are assigned to teams in local games
 *
 * @param team Team to receive point
 */
void LocalGame::score (unsigned char team) {

	// Do nothing

	return;

}


/**
 * Set the checkpoint
 *
 * @param gridX X-coordinate (in tiles) of the checkpoint
 * @param gridY Y-coordinate (in tiles) of the checkpoint
 */
void LocalGame::setCheckpoint (unsigned char gridX, unsigned char gridY) {

	checkX = gridX;
	checkY = gridY;

	return;

}

