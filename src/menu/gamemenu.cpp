
/**
 *
 * @file gamemenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd of August 2005: Created menu.c
 * - 3rd of February 2009: Renamed menu.c to menu.cpp
 * - 18th July 2009: Created menugame.cpp from parts of menu.cpp
 * - 26th July 2009: Renamed menugame.cpp to gamemenu.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the running of the menus used to create a new game.
 *
 */


#include "menu.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "jj1/save/jj1save.h"
#include "loop.h"
#include "util.h"
#include "io/log.h"
#include <memory>

static bool getSaveData(int slot, int &lvl, int &plnt, int &dffclty) {
	// load save game
	char* fileName = createString("SAVE.0");
	fileName[5] += slot;
	auto save = std::make_unique<JJ1Save>(fileName);
	delete[] fileName;

	if(save->valid) {
		lvl = save->level;
		plnt = save->planet;
		dffclty = save->difficulty;

		return true;
	}

	return false;
}


/**
 * Create the game menu.
 *
 * @param file File containing menu graphics
 */
GameMenu::GameMenu (File *file) {

	difficulty = 0;

	// Load the difficulty graphics
	file->loadPalette(menuPalette);
	difficultyScreen = file->loadSurface(SW, SH);
	enableColorKey(difficultyScreen, 0);

	// Load the episode pictures (max. 10 episodes + bonus level)

	// Load their palette
	file->loadPalette(palette);

	// Generate a greyscale mapping
	for (int i = 0; i < MAX_PALETTE_COLORS; i++) {

		int col = ((palette[i].r >> 1) + (palette[i].g << 1) + (palette[i].b >> 1)) >> 3;

		if (col > 79) col = 79;

		greyPalette[i].r = greyPalette[i].g = greyPalette[i].b = col;

	}

	episodes = MAX_EPISODES;

	for (int i = 0; i < MAX_EPISODES; i++) {

		episodeScreens[i] = file->loadSurface(134, 110);

		if (file->tell() >= file->getSize()) {

			episodes = ++i;

			for (; i < MAX_EPISODES; i++) {

				unsigned char pixel = 0;
				episodeScreens[i] = createSurface(&pixel, 1, 1);

			}

		}

	}

	fileMenu = new FileMenu();

}


/**
 * Delete the game menu.
 */
GameMenu::~GameMenu () {

	for (int i = 0; i < MAX_EPISODES; i++) SDL_FreeSurface(episodeScreens[i]);

	SDL_FreeSurface(difficultyScreen);

	delete fileMenu;

}


/**
 * Create and play a new game.
 *
 * @param mode Game mode
 * @param firstLevel First level's file name
 *
 * @return Error code
 */
int GameMenu::playNewGame (GameModeType mode, char* firstLevel) {

	Game* game;
	int ret;

	playConfirmSound();

	if (mode == M_SINGLE) {

		try {

			game = new LocalGame(firstLevel, difficulty);

		} catch (int e) {

			if (message("COULD NOT START GAME") == E_QUIT) return E_QUIT;

			return e;

		}

	} else {

		try {

			game = new ServerGame(mode, firstLevel, difficulty);

		} catch (int e) {

			if (message("COULD NOT CREATE SERVER") == E_QUIT) return E_QUIT;

			return e;

		}

	}


	// Play the level(s)

	ret = game->play();

	delete game;

	if (ret != E_QUIT) playMusic("MENUSNG.PSM");

	switch (ret) {

		case E_LOAD0:
		case E_LOAD1:
		case E_LOAD2:
		case E_LOAD3:
			{
				// FIXME: Rewrite load logic, save state in memory

				int lvl, plnt, dffclty;
				if (getSaveData(ret - E_LOAD0, lvl, plnt, dffclty)) {
					char* firstLevel = createFileName("LEVEL", lvl, plnt);
					difficulty = dffclty;
					ret = playNewGame(M_SINGLE, firstLevel); // recursive call
					delete[] firstLevel;
				} else
					ret = E_NONE;

				return ret;
			}

		case E_QUIT:

			return E_QUIT;

		case E_FILE:

			return message("FILE NOT FOUND OR CORRUPTED");

	}

	return E_NONE;

}


/**
 * Run the new game difficulty menu.
 *
 * @param mode Game mode
 * @param firstLevel First level's file name
 *
 * @return Error code
 */
int GameMenu::newGameDifficulty (GameModeType mode, char* firstLevel) {

	const char *options[4] = {"easy", "medium", "hard", "turbo"};
	SDL_Rect src, dst;
	int x, y, count;

	video.setPalette(menuPalette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_UP)) difficulty = (difficulty + 3) % 4;

		if (controls.release(C_DOWN)) difficulty = (difficulty + 1) % 4;

		if (controls.release(C_ENTER)) return playNewGame(mode, firstLevel);

		if (controls.getCursor(x, y)) {

			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;

			x -= canvasW >> 2;
			y -= (canvasH >> 1) - 32;

			if ((x >= 0) && (x < 256) && (y >= 0) && (y < 64)) {

				difficulty = y >> 4;

				if (controls.wasCursorReleased()) return playNewGame(mode, firstLevel);

			}

		}

		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);

		for (count = 0; count < 4; count++) {

			if (count == difficulty) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], canvasW >> 2,
				(canvasH >> 1) + (count << 4) - 32);

			if (count == difficulty) fontmn2->restorePalette();

		}

		src.x = (difficulty & 1) * 160;
		src.y = (difficulty & 2) * 50;
		src.w = 160;
		src.h = 100;
		dst.x = (canvasW >> 1) - 40;
		dst.y = (canvasH >> 1) - 50;
		SDL_BlitSurface(difficultyScreen, &src, canvas, &dst);

		showEscString();

	}

	return E_NONE;

}


/**
 * Run the new game difficulty menu.
 *
 * @param mode Game mode
 * @param levelNum First level's number
 * @param levelNum First level's world number
 *
 * @return Error code
 */
int GameMenu::newGameDifficulty (GameModeType mode, int levelNum, int worldNum) {

	char* firstLevel;
	int ret;

	if (levelNum == -1) firstLevel = createFileName("BONUSMAP", worldNum);
	else firstLevel = createFileName("LEVEL", levelNum, worldNum);

	ret = newGameDifficulty(mode, firstLevel);

	delete[] firstLevel;

	return ret;

}


/**
 * Run the game loading menu.
 *
 * @return Error code
 */
int GameMenu::loadGame () {
	int ret;

	while (true) {
		ret = fileMenu->main(false, true);
		if (ret < 0) return ret;

		if (ret == 4) {
			while (true) {
				ret = loadGameCustom();
				if (ret == E_QUIT) return E_QUIT; // return to main menu
				if (ret <= 0) break; // return to load menu
			}
		} else {
			// load save game
			int lvl, plnt, dffclty;
			if (getSaveData(ret, lvl, plnt, dffclty)) {
				char* firstLevel = createFileName("LEVEL", lvl, plnt);
				difficulty = dffclty;
				ret = playNewGame(M_SINGLE, firstLevel);
				delete[] firstLevel;
			} else
				ret = E_NONE;

			break;
		}
	}

	return ret;
}


/**
 * Run the custom game loading menu.
 *
 * @return Error code
 */
int GameMenu::loadGameCustom () {

	int option, worldNum, levelNum, x, y;

	worldNum = levelNum = option = 0;

	video.setPalette(menuPalette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_UP)) option ^= 1;

		if (controls.release(C_DOWN)) option ^= 1;

		if (controls.release(C_LEFT)) {

			if (option) levelNum = ((levelNum + MAX_EPISODES) % MAX_EPISODES) - 1;
			else worldNum = (worldNum + 999) % 1000;

		}

		if (controls.release(C_RIGHT)) {

			if (option) levelNum = ((levelNum + 2) % MAX_EPISODES) - 1;
			else worldNum = (worldNum + 1) % 1000;

		}

		if (controls.release(C_ENTER)) {

			playConfirmSound();

			if (newGameDifficulty(M_SINGLE, levelNum, worldNum) == E_QUIT) return E_QUIT;

			video.setPalette(menuPalette);

		}

		if (controls.getCursor(x, y)) {

			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;

			if (y < (canvasH >> 1)) option = 0;
			else option = 1;

		}


		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(15);

		if (option == 0) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("choose world:", 32, canvasH / 3);
		fontmn2->showNumber(worldNum, 208, canvasH / 3);

		if (option == 0) fontmn2->restorePalette();
		else fontmn2->mapPalette(240, 8, 114, 16);

		fontmn2->showString("choose level:", 32, (canvasH << 1) / 3);
		if (levelNum >= 0) fontmn2->showNumber(levelNum, 208, (canvasH << 1) / 3);
		else fontmn2->showString("bonus", 172, (canvasH << 1) / 3);

		if (option != 0) fontmn2->restorePalette();

		showEscString();

	}

	return E_NONE;

}


/**
 * Directly load level.
 *
 * @return Error code
 */
int GameMenu::loadGame (int levelNum, int worldNum) {

	if (newGameDifficulty(M_SINGLE, levelNum, worldNum) == E_QUIT) return E_QUIT;

	return E_NONE;

}


/**
 * Run the new game level selection menu.
 *
 * @param mode Game mode
 *
 * @return Error code
 */
int GameMenu::newGameLevel (GameModeType mode) {

	char* fileName;
	int ret;

	fileName = createString("level0.000");

	ret = E_NONE;

	while (true) {

		ret = textInput("level file name:", fileName);

		if (ret < 0) break;

		ret = newGameDifficulty(mode, fileName);

		if (ret < 0) break;

	}

	delete[] fileName;

	return ret;

}


/**
 * Run the appropriate menu for the given episode selection.
 *
 * @param mode Game mode
 * @param episode Episode number
 *
 * @return Error code
 */
int GameMenu::selectEpisode (GameModeType mode, int episode) {

	playConfirmSound();

	if (episode < 10) {

		int worldNum = episodeToWorld(episode);

		if (newGameDifficulty(mode, 0, worldNum) == E_QUIT) return E_QUIT;

	} else if (episode == 10) {

		if (newGameDifficulty(mode, -1, 0) == E_QUIT) return E_QUIT;

	} else {

		if (newGameLevel(mode) == E_QUIT) return E_QUIT;

	}

	video.setPalette(palette);

	return E_NONE;

}


/**
 * Run the new game episode menu.
 *
 * @param mode Game mode
 *
 * @return Error code
 */
int GameMenu::newGameEpisode (GameModeType mode) {
	const char *episodeTag[12] = { "1", "2", "3", "4", "5", "6",
		"a", "b", "c", "x", "z", " " };
	const char *episodeTitle[12] = { "turtle terror", "ballistic bunny",
		"rabbits revenge", "gene machine", "the chase is on",
		"the final clash", "outta dis world", "turtle soup",
		"wild wabbit", "holiday hare", "bonus levels", "specific level" };

	bool exists[12];
	char *check;
	SDL_Rect dst;
	int episode, count, x, y;

	video.setPalette(palette);

	for (count = 0; count < 10; count++) {

		check = createFileName("LEVEL", 0, episodeToWorld(count));
		exists[count] = fileExists(check, PATH_TYPE_GAME);
		delete[] check;

		if (exists[count]) video.restoreSurfacePalette(episodeScreens[count]);
		else setLogicalPalette(episodeScreens[count], greyPalette, 0, MAX_PALETTE_COLORS);

	}

	if (mode == M_SINGLE) {

		check = createFileName("BONUSMAP", 0);
		exists[10] = fileExists(check, PATH_TYPE_GAME);
		delete[] check;

	} else exists[10] = false;

	exists[MAX_EPISODES] = true; // always possible to load custom level

	episode = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_UP)) episode = (episode + MAX_EPISODES) % 12;

		if (controls.release(C_DOWN)) episode = (episode + 1) % 12;

		if (controls.release(C_ENTER) && exists[episode]) {

			count = selectEpisode(mode, episode);

			if (count < 0) return count;

		}

		if (controls.getCursor(x, y)) {

			if ((x >= canvasW - 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;

			x -= canvasW >> 3;
			y -= (canvasH >> 1) - 92;

			if ((x >= 0) && (x < 256) && (y >= 0) && (y < 192)) {

				episode = y >> 4;

				if (controls.wasCursorReleased() && exists[episode]) {

					count = selectEpisode(mode, episode);

					if (count < 0) return count;

				}

			}

		}


		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);

		dst.x = canvasW - 144;
		dst.y = (canvasH - 110) >> 1;

		if ((episode < episodes - 1) || (episode < 6)) {

			SDL_BlitSurface(episodeScreens[episode], NULL, canvas, &dst);

		} else if ((episode == 10) && (episodes > 6)) {

			SDL_BlitSurface(episodeScreens[episodes - 1], NULL, canvas, &dst);

		}

		int episodeX = canvasW >> 3;

		for (count = 0; count < 12; count++) {

			if (count == episode) {

				// black on white
				fontmn2->mapPalette(240, 8, 79, -80);
				drawRect(episodeX - 2, (canvasH >> 1) + (count << 4) - 94,
					160, 15, 79);

			} else if (!exists[count])
				fontmn2->mapPalette(240, 8, 94, -16);

			// align both separately
			fontmn2->showString(episodeTag[count], episodeX - (count < 6 ? 2 : 0),
				(canvasH >> 1) + (count << 4) - 92);
			fontmn2->showString(episodeTitle[count], episodeX + 16,
				(canvasH >> 1) + (count << 4) - 92);

			if ((count == episode) || (!exists[count]))
				fontmn2->mapPalette(240, 8, 9, 80);

		}

		fontbig->showString(ESCAPE_STRING, canvasW - 100, canvasH - 12);

	}

	return E_NONE;

}


/**
 * Run the game joining menu.
 *
 * @return Error code
 */
int GameMenu::joinGame () {

	Game* game;
	int ret;

	ret = textInput("ip address:", netAddress, true);

	if (ret < 0) return ret;

	try {

		game = new ClientGame(netAddress);

	} catch (int e) {

		switch (e) {

			case E_N_SOCKET:

				if (message("SOCKET ERROR") == E_QUIT) return E_QUIT;

				break;

			case E_N_ADDRESS:

				if (message("INVALID ADDRESS") == E_QUIT) return E_QUIT;

				break;

			case E_N_CONNECT:

				if (message("COULD NOT CONNECT") == E_QUIT) return E_QUIT;

				break;

			case E_TIMEOUT:

				if (message("OPERATION TIMED OUT") == E_QUIT) return E_QUIT;

				break;

			case E_DATA:

				if (message("INCORRECT DATA\nRECEIVED") == E_QUIT) return E_QUIT;

				break;

			case E_VERSION:

				if (message("WRONG SERVER VERSION") == E_QUIT) return E_QUIT;

				break;

			case E_RETURN:
			case E_QUIT:

				break;

			default:

				if (message("COULD NOT COMPLETE CONNECTION") == E_QUIT) return E_QUIT;

				break;

		}

		return e;

	}


	// Play the level(s)

	ret = game->play();

	delete game;

	if (ret != E_QUIT) playMusic("MENUSNG.PSM");

	switch (ret) {

		case E_QUIT:

			return E_QUIT;

		case E_FILE:

			return message("FILE NOT FOUND");

		case E_N_DISCONNECT:

			return message("DISCONNECTED");

	}

	return E_NONE;

}


/**
 * Run the new game menu.
 *
 * @return Error code
 */
int GameMenu::newGame () {

#if (defined USE_SOCKETS) || (defined USE_SDL_NET)
	const char *newGameOptions[6] = {"new single player game", "new co-op game",
		"new battle", "new team battle", "new race", "join game"};
	int option = 0;

	while (true) {

		video.setPalette(menuPalette);

		int ret = generic(newGameOptions, 6, option);

		if (ret == E_QUIT) return E_QUIT;
		if (ret < 0) return E_NONE;

		if (option == 5) {

			if (joinGame() == E_QUIT) return E_QUIT;

		} else {

			if (newGameEpisode(GameModeType(option)) == E_QUIT) return E_QUIT;

		}

	}

	return E_NONE;
#else
	return newGameEpisode(M_SINGLE);
#endif

}
