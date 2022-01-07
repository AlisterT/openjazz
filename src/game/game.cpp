
/**
 *
 * @file game.cpp
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

#include "io/gfx/video.h"
#include "io/sound.h"
#include "jj1/bonuslevel/jj1bonuslevel.h"
#include "jj1/level/jj1level.h"
#include "jj1/planet/jj1planet.h"
#include "jj2/level/jj2level.h"
#include "player/player.h"
#include "util.h"

#include <string.h>


/**
 * Create base game
 */
Game::Game () {

	levelFile = NULL;
	levelType = LT_JJ1;

	players = NULL;
	baseLevel = NULL;

	return;

}


/**
 * Destroy game
 */
Game::~Game () {

	if (levelFile) delete[] levelFile;

	if (players) delete[] players;
	localPlayer = NULL;

	return;

}


/**
 * Check the if a file name corresponds to a type
 *
 * @param fileName The name of the file
 * @param type The type identifier in lower case characters
 * @param typeLength The length of the type identifier string
 *
 * @return Whether or not the file name corresponds to the type
 */
bool Game::isFileType (const char *fileName, const char *type, int typeLength) {

	int i;

	for (i = 0; i < typeLength; i++) {

		if ((fileName[i] != type[i]) && (fileName[i] != type[i] - 32)) return false;

	}

	return true;
}


/**
 * Create a new game mode
 *
 * @param modeType The mode to create
 *
 * @return The new game mode (NULL on failure)
 */
GameMode* Game::createMode (GameModeType modeType) {

	switch (modeType) {

		case M_SINGLE:

			return new SingleGameMode();

		case M_COOP:

			return new CoopGameMode();

		case M_BATTLE:

			return new BattleGameMode();

		case M_TEAMBATTLE:

			return new TeamBattleGameMode();

		case M_RACE:

			return new RaceGameMode();

	}

	return NULL;

}


/**
 * Get the game's mode
 *
 * @return The game's mode
 */
GameMode* Game::getMode () {

	return mode;

}


/**
 * Get the game's difficulty
 *
 * @return The game's difficulty
 */
int Game::getDifficulty () {

	return difficulty;

}


/**
 * Set the game's difficulty
 */
void Game::setDifficulty (int diff) {

	difficulty = diff;

	return;

}


/**
 * Play a level.
 *
 * @return Error code
 */
int Game::playLevel (char* fileName, bool intro, bool checkpoint) {

	bool multiplayer;
	int ret;

	multiplayer = (mode->getMode() != M_SINGLE);

	if (isFileType(fileName, "macro", 5)) {

		// Load and play the level

		try {

			baseLevel = level = new JJ1DemoLevel(this, fileName);

		} catch (int e) {

			return e;

		}

		ret = level->play();

		delete level;
		baseLevel = level = NULL;

	} else if (levelType == LT_JJ1BONUS) {

		JJ1BonusLevel *bonus;

		try {

			baseLevel = bonus = new JJ1BonusLevel(this, fileName, multiplayer);

		} catch (int e) {

			return e;

		}

		ret = bonus->play();

		delete bonus;
		baseLevel = NULL;

	} else if (levelType == LT_JJ2) {

		try {

			baseLevel = jj2Level = new JJ2Level(this, fileName, checkpoint, multiplayer);

		} catch (int e) {

			return e;

		}

		ret = jj2Level->play();

		delete jj2Level;
		baseLevel = jj2Level = NULL;

	} else {

		try {

			baseLevel = level = new JJ1Level(this, fileName, checkpoint, multiplayer);

		} catch (int e) {

			return e;

		}

		if (intro) {

			JJ1Planet *planet;
			char *planetFileName = NULL;

			planetFileName = createFileName("PLANET", level->getWorld());

			try {

				planet = new JJ1Planet(planetFileName, planetId);

			} catch (int e) {

				planet = NULL;

			}

			delete[] planetFileName;

			if (planet) {

				if (planet->play() == E_QUIT) {

					delete planet;
					delete level;

					return E_QUIT;

				}

				planetId = planet->getId();

				delete planet;

			}

		}

		ret = level->play();

		delete level;
		baseLevel = level = NULL;

	}

	return ret;

}


/**
 * Determine the type of the specified level file.
 *
 * @return Level type
 */
LevelType Game::getLevelType (const char* fileName) {

	int length;

	length = strlen(fileName);

	if ((length > 4) && isFileType(fileName + length - 4, ".j2l", 4)) return LT_JJ2;
	if (isFileType(fileName, "bonusmap", 8)) return LT_JJ1BONUS;
	return LT_JJ1;

}


/**
 * Play a level.
 *
 * @return Error code
 */
int Game::playLevel (char* fileName) {

	levelType = getLevelType(fileName);

	return playLevel(fileName, false, false);

}


/**
 * Play the game
 *
 * @return Error code
 */
int Game::play () {

	bool multiplayer;
	bool checkpoint;
	int ret;

	multiplayer = (mode->getMode() != M_SINGLE);
	checkpoint = false;
	planetId = -1;

	// Play the level(s)
	while (true) {

		if (!levelFile) return E_NONE;

		sendTime = checkTime = 0;

		// Load and play the level

		ret = playLevel(levelFile, !multiplayer, checkpoint);

		if (ret <= 0) return ret;

		if (levelFile && isFileType(levelFile, "bonusmap", 8)) {

			if (ret == WON) {

				char *fileName;

				// Go to next level
				fileName = createFileName("BONUSMAP", (levelFile[10] * 10) + levelFile[11] - 527);
				setLevel(fileName);
				delete[] fileName;

			}

		} else {

			if (ret == WON) {

				// Won the level

				// Do not use old level's checkpoint coordinates
				checkpoint = false;

			} else {

				// Lost the level

				if (!localPlayer->getLives()) return E_NONE;

				// Use checkpoint coordinates
				checkpoint = true;

			}

		}

	}

	return E_NONE;

}


/**
 * Move the viewport towards the exit sign
 *
 * @param change Distance to move
 */
void Game::view (int change) {

	if (TTOF(checkX) > viewX + (canvasW << 9) + change) viewX += change;
	else if (TTOF(checkX) < viewX + (canvasW << 9) - change) viewX -= change;

	if (TTOF(checkY) > viewY + (canvasH << 9) + change) viewY += change;
	else if (TTOF(checkY) < viewY + (canvasH << 9) - change) viewY -= change;

	return;

}


/**
 * Make a player restart the level from the beginning/last checkpoint
 *
 * @param player Player to reset
 */
void Game::resetPlayer (Player *player) {

	player->reset(checkX, checkY);

	return;

}


/**
 * Create a level player
 *
 * @param player Player for which to create the level player
 */
void Game::addLevelPlayer (Player *player) {

	int count;

	if (level) {

		Anim* pAnims[JJ1PANIMS];

		for (count = 0; count < JJ1PANIMS; count++) pAnims[count] = level->getPlayerAnim(count);

		player->createLevelPlayer(levelType, pAnims, NULL, checkX, checkY);

	} else if (jj2Level) {

		Anim* pAnims[JJ2PANIMS];
		Anim* pFlippedAnims[JJ2PANIMS];

		for (count = 0; count < JJ2PANIMS; count++) {

			pAnims[count] = jj2Level->getPlayerAnim(0, count, false);
			pFlippedAnims[count] = jj2Level->getPlayerAnim(0, count, true);

		}

		player->createLevelPlayer(levelType, pAnims, pFlippedAnims, checkX, checkY);

	}

	return;

}

