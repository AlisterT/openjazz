
/*
 *
 * game.cpp
 *
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 3rd June 2009: Created network.cpp from parts of game.cpp
 * 18th July 2009: Created servergame.cpp from parts of game.cpp
 * 18th July 2009: Created clientgame.cpp from parts of game.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "level/level.h"
#include "player/player.h"
#include "scene.h"


Game::Game () {

	levelFile = NULL;

	players = NULL;

	return;

}


Game::Game (char *firstLevel, int gameDifficulty) {

	levelFile = createString(firstLevel);
	difficulty = gameDifficulty;

	gameMode = NULL;

	// Create the player
	nPlayers = 1;
	localPlayer = players = new Player[1];
	localPlayer->init(characterName, NULL, 0);

	return;

}


Game::~Game () {

	if (levelFile) delete[] levelFile;

	if (players != NULL) delete[] players;
	localPlayer = NULL;

	return;

}


int Game::setLevel (char *fileName) {

	if (levelFile) delete[] levelFile;

	if (!fileName) levelFile = NULL;
	else levelFile = createString(fileName);

	return E_NONE;

}


int Game::play () {

	Scene * scene;
	bool checkpoint;
	int ret;

	checkpoint = false;

	// Play the level(s)
	while (true) {

		sendTime = checkTime = 0;

		// Load the level

		try {

			level = new Level(levelFile, difficulty, checkpoint);

		} catch (int e) {

			return e;

		}

		ret = level->play();

		switch (ret) {

			case E_NONE: // Quit game

				delete level;

				playMusic("menusng.psm");

				return E_NONE;

			case WON: // Completed level

				// If there is no next level, load and play the cutscene
				if (!levelFile) {

					scene = level->createScene();

					delete level;

					scene->play();

					delete scene;

					return E_NONE;

				}

				// Do not use old level's checkpoint coordinates
				checkpoint = false;

				break;

			case LOST: // Lost level

				if (!localPlayer->getLives()) {

					delete level;

					return E_NONE; // Not really a success...

				}

				// Use checkpoint coordinates
				checkpoint = true;

				break;

			default: // Error

				delete level;

				return ret;

		}

		// Unload the previous level
		delete level;

	}

	return E_NONE;

}


void Game::view () {

	// Move the viewport towards the exit sign

	if (TTOF(checkX) > viewX + (viewW << 9) + (160 * mspf)) viewX += 160 * mspf;
	else if (TTOF(checkX) < viewX + (viewW << 9) - (160 * mspf))
		viewX -= 160 * mspf;

	if (TTOF(checkY) > viewY + (viewH << 9) + (160 * mspf)) viewY += 160 * mspf;
	else if (TTOF(checkY) < viewY + (viewH << 9) - (160 * mspf))
		viewY -= 160 * mspf;

	return;

}


void Game::send (unsigned char *buffer) {

	// Do nothing

	return;

}


int Game::playFrame (int ticks) {

	// Do nothing

	return E_NONE;

}


void Game::score (unsigned char team) {

	// Do nothing

	return;

}


void Game::setCheckpoint (unsigned char gridX, unsigned char gridY) {

	checkX = gridX;
	checkY = gridY;

	return;

}


void Game::resetPlayer (Player *player) {

	player->reset();
	player->setPosition(TTOF(checkX), TTOF(checkY));

	return;

}


