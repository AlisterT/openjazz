
/**
 *
 * @file localgame.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c and menu.c
 * - 3rd of February 2009: Renamed level.c to level.cpp and menu.c to menu.cpp
 * - 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * - 3rd June 2009: Created network.cpp from parts of game.cpp
 * - 18th July 2009: Created servergame.cpp from parts of game.cpp
 * - 18th July 2009: Created clientgame.cpp from parts of game.cpp
 * - 3rd October 2010: Created localgame.cpp from parts of game.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#include "game.h"
#include "gamemode.h"

#include "player/player.h"
#include "setup.h"
#include "util.h"


/**
 * Create a single-player local game
 *
 * @param firstLevel File name of the first level to play
 * @param gameDifficulty Difficulty setting
 */
LocalGame::LocalGame (const char *firstLevel, int gameDifficulty) {

	levelFile = createString(firstLevel);
	levelType = getLevelType(firstLevel);

	difficulty = gameDifficulty;

	mode = new SingleGameMode();

	// Create the player
	nPlayers = 1;
	localPlayer = players = new Player[1];
	localPlayer->init(this, setup.characterName, NULL, 0);

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

	if (fileName) {

		levelFile = createString(fileName);
		levelType = getLevelType(fileName);

	} else levelFile = NULL;

	return E_NONE;

}


/**
 * No data is sent in local games
 *
 * @param buffer Data that will not be sent. First byte indicates length.
 */
void LocalGame::send (unsigned char *buffer) {

	(void)buffer;

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

	(void)ticks;

	// Do nothing

	return E_NONE;

}


/**
 * No points are assigned to teams in local games
 *
 * @param team Team to receive point
 */
void LocalGame::score (unsigned char team) {

	(void)team;

	// Do nothing

	return;

}


/**
 * Set the checkpoint
 *
 * @param gridX X-coordinate (in tiles) of the checkpoint
 * @param gridY Y-coordinate (in tiles) of the checkpoint
 */
void LocalGame::setCheckpoint (int gridX, int gridY) {

	checkX = gridX;
	checkY = gridY;

	return;

}

