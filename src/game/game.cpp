
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
 * 3rd October 2010: Created localgame.cpp from parts of game.cpp
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
 */


#include "game.h"
#include "gamemode.h"

#include "bonus/bonus.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "jj2level/jj2level.h"
#include "level/level.h"
#include "planet/planet.h"
#include "player/levelplayer.h"
#include "util.h"

#include <string.h>


/**
 * Create base game
 */
Game::Game () {

	levelFile = NULL;

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

	// Load and play the level

	if (!strncasecmp(fileName, F_BONUSMAP, 8)) {

		Bonus *bonus;

		try {

			baseLevel = bonus = new Bonus(this, fileName, multiplayer);

		} catch (int e) {

			return e;

		}

		ret = bonus->play();

		delete bonus;
		baseLevel = NULL;

	} else if (!strncasecmp(fileName, "MACRO", 5)) {

		try {

			baseLevel = level = new DemoLevel(this, fileName);

		} catch (int e) {

			return e;

		}

		ret = level->play();

		delete level;
		baseLevel = level = NULL;

	} else if (!strcasecmp(fileName + strlen(fileName) - 4, ".j2l")) {

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

			baseLevel = level = new Level(this, fileName, checkpoint, multiplayer);

		} catch (int e) {

			return e;

		}

		if (intro) {

			Planet *planet;
			char *planetFileName = NULL;

			planetFileName = createFileName(F_PLANET, level->getWorld());

			try {

				planet = new Planet(planetFileName, planetId);

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

		if (ret <= 0) {

			if (ret == E_NONE) playMusic("menusng.psm");

			return ret;

		}

		if (levelFile && !strncasecmp(levelFile, F_BONUSMAP, 8)) {

			if (ret == WON) {

				char *fileName;

				// Go to next level
				fileName = createFileName(F_BONUSMAP, (levelFile[10] * 10) + levelFile[11] - 527);
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

