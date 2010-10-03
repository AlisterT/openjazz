
/**
 *
 * @file game.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 3rd June 2009: Created network.cpp from parts of game.cpp
 * 18th July 2009: Created servergame.cpp from parts of game.cpp
 * 18th July 2009: Created clientgame.cpp from parts of game.cpp
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

#include "bonus/bonus.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "jj2level/jj2level.h"
#include "level/level.h"
#include "planet/planet.h"
#include "player/bonusplayer.h"
#include "player/levelplayer.h"
#include "util.h"

#include <string.h>


/**
 * Create base game
 */
Game::Game () {

	levelFile = NULL;

	players = NULL;

	return;

}


/**
 * Create a new game
 *
 * @param firstLevel File name of the first level to play
 * @param gameDifficulty Difficulty setting
 */
Game::Game (char *firstLevel, int gameDifficulty) {

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
 * Destroy game
 */
Game::~Game () {

	if (levelFile) delete[] levelFile;

	if (players) delete[] players;
	localPlayer = NULL;

	return;

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
 * Set the next level
 *
 * @param fileName The file name of the next level
 *
 * @return Error code
 */
int Game::setLevel (char *fileName) {

	if (levelFile) delete[] levelFile;

	if (fileName) levelFile = createString(fileName);
	else levelFile = NULL;

	return E_NONE;

}


/**
 * Play the game
 *
 * @return Error code
 */
int Game::play () {

	Planet* planet;
	Bonus* bonus;
	char* fileName;
	bool multiplayer;
	bool checkpoint;
	int ret;
	int planetId;

	multiplayer = (mode->getMode() != M_SINGLE);
	checkpoint = false;
	planetId = -1;

	// Play the level(s)
	while (true) {

		if (!levelFile) return E_NONE;

		sendTime = checkTime = 0;


		// Load and play the level

		if (!strncasecmp(levelFile, F_BONUSMAP, 8)) {

			try {

				baseLevel = bonus = new Bonus(levelFile, difficulty, multiplayer);

			} catch (int e) {

				return e;

			}

			ret = bonus->play();

			if (ret <= 0) {

				delete bonus;
				baseLevel = NULL;

				if (ret == E_NONE) playMusic("menusng.psm");

				return ret;

			} else if (ret == WON) {

				// Go to next level
				fileName = createFileName(F_BONUSMAP, (levelFile[10] * 10) + levelFile[11] - 527);
				setLevel(fileName);
				delete[] fileName;

			}

			delete bonus;
			baseLevel = NULL;

		} else if (!strcasecmp(levelFile + strlen(levelFile) - 4, ".j2l")) {

			try {

				baseLevel = jj2Level = new JJ2Level(levelFile, difficulty, checkpoint, multiplayer);

			} catch (int e) {

				return e;

			}

			ret = jj2Level->play();

			if (ret <= 0) {

				delete jj2Level;
				baseLevel = jj2Level = NULL;

				if (ret == E_NONE) playMusic("menusng.psm");

				return ret;

			} else if (ret == WON) {

				// Won the level

				// Do not use old level's checkpoint coordinates
				checkpoint = false;

			} else {

				// Lost the level

				if (!localPlayer->getLives()) return E_NONE;

				// Use checkpoint coordinates
				checkpoint = true;


			}

			delete jj2Level;
			baseLevel = jj2Level = NULL;

		} else {

			try {

				baseLevel = level = new Level(levelFile, difficulty, checkpoint, multiplayer);

			} catch (int e) {

				return e;

			}

			if (!multiplayer) {

				planet = NULL;
				fileName = createFileName(F_PLANET, level->getWorld());

				try {

					planet = new Planet(fileName, planetId);

				} catch (int e) {

					// Do nothing

				}

				delete[] fileName;

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

			if (ret <= 0) {

				delete level;
				baseLevel = level = NULL;

				if (ret == E_NONE) playMusic("menusng.psm");

				return ret;

			} else if (ret == WON) {

				// Won the level

				// Do not use old level's checkpoint coordinates
				checkpoint = false;

			} else {

				// Lost the level

				if (!localPlayer->getLives()) return E_NONE;

				// Use checkpoint coordinates
				checkpoint = true;


			}

			delete level;
			baseLevel = level = NULL;

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
 * No data is sent in single-player mode
 *
 * @param buffer Data that will not be sent. First byte indicates length.
 */
void Game::send (unsigned char *buffer) {

	// Do nothing

	return;

}


/**
 * Game iteration
 *
 * @param ticks Current time
 *
 * @return Error code
 */
int Game::step (unsigned int ticks) {

	// Do nothing

	return E_NONE;

}


/**
 * Assign point to team
 *
 * @param team Team to receive point
 */
void Game::score (unsigned char team) {

	// Do nothing

	return;

}


/**
 * Set the checkpoint
 *
 * @param gridX X-coordinate (in tiles) of the checkpoint
 * @param gridY Y-coordinate (in tiles) of the checkpoint
 */
void Game::setCheckpoint (unsigned char gridX, unsigned char gridY) {

	checkX = gridX;
	checkY = gridY;

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
 * Re-create a player's level player
 *
 * @param player Player to reset
 * @param levelType Type of level (and, consequently, type of level player)
 * @param anims New level player's animations
 */
void Game::resetPlayer (Player *player, LevelType levelType, Anim** anims) {

	Anim* pAnims[PANIMS];
	int count;

	if (anims) {

		player->reset(levelType, anims, checkX, checkY);

	} else if (level) {

		for (count = 0; count < PANIMS; count++) pAnims[count] = level->getAnim(0);
		player->reset(levelType, pAnims, checkX, checkY);

	} else if (jj2Level) {

		pAnims[0] = jj2Level->getAnim(54, 0, false);
		pAnims[1] = jj2Level->getAnim(54, 0, true);

		player->reset(levelType, pAnims, checkX, checkY);

	}

	return;

}

