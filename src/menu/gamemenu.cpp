
/*
 *
 * gamemenu.cpp
 *
 * 18th July 2009: Created menugame.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menugame.cpp to gamemenu.cpp
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
#include "loop.h"
#include "util.h"


GameMenu::GameMenu (File *file) {

	unsigned char pixel;
	int count, col;


	// Load the difficulty graphics
	file->loadPalette(menuPalette);
	difficultyScreen = file->loadSurface(SW, SH);
	SDL_SetColorKey(difficultyScreen, SDL_SRCCOLORKEY, 0);

	// Default difficulty setting
	difficulty = 1;


	// Load the episode pictures (max. 10 episodes + bonus level)

	// Load their palette
	file->loadPalette(palette);

	// Generate a greyscale mapping
	for (count = 0; count < 256; count++) {

		col = ((palette[count].r >> 1) + (palette[count].g << 1) + (palette[count].b >> 1)) >> 3;

		if (col > 79) col = 79;

		greyPalette[count].r = greyPalette[count].g = greyPalette[count].b = col;

	}

	episodes = 11;

	for (count = 0; count < 11; count++) {

		episodeScreens[count] = file->loadSurface(134, 110);

		if (file->tell() >= file->getSize()) {

			episodes = ++count;

			for (; count < 11; count++) {

				pixel = 0;
				episodeScreens[count] = createSurface(&pixel, 1, 1);

			}

		}

	}

	return;

}


GameMenu::~GameMenu () {

	int count;

	for (count = 0; count < 11; count++) SDL_FreeSurface(episodeScreens[count]);

	SDL_FreeSurface(difficultyScreen);

	return;

}


int GameMenu::newGameDifficulty (GameModeType mode, char* firstLevel) {

	const char *options[4] = {"easy", "medium", "hard", "turbo"};
	SDL_Rect src, dst;
	int count;

	video.setPalette(menuPalette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		SDL_Delay(T_FRAME);

		clearScreen(0);

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

		if (controls.release(C_UP)) difficulty = (difficulty + 3) % 4;

		if (controls.release(C_DOWN)) difficulty = (difficulty + 1) % 4;

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			if (mode == M_SINGLE) {

				try {

					game = new Game(firstLevel, difficulty);

				} catch (int e) {

					message("COULD NOT START GAME");

					return e;

				}

			} else {

				try {

					game = new ServerGame(mode, firstLevel, difficulty);

				} catch (int e) {

					message("COULD NOT CREATE SERVER");

					return e;

				}

			}


			// Play the level(s)

			switch (game->play()) {

				case E_QUIT:

					delete game;

					return E_QUIT;

				case E_FILE:

					message("FILE NOT FOUND");

					break;

			}

			delete game;

			return E_NONE;

		}

	}

	return E_NONE;

}


int GameMenu::newGameDifficulty (GameModeType mode, int levelNum, int worldNum) {

	char* firstLevel;
	int ret;

	if (levelNum == -1) firstLevel = createFileName(F_BONUSMAP, worldNum);
	else firstLevel = createFileName(F_LEVEL, levelNum, worldNum);

	ret = newGameDifficulty(mode, firstLevel);

	delete[] firstLevel;

	return ret;

}


int GameMenu::loadGame () {

	// TODO: Actual loading of saved games

	int option, worldNum, levelNum;

	worldNum = levelNum = option = 0;

	video.setPalette(menuPalette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		SDL_Delay(T_FRAME);

		clearScreen(15);

		if (option == 0) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("choose world:", 32, canvasH / 3);
		fontmn2->showNumber(worldNum, 208, canvasH / 3);

		if (option == 0) fontmn2->restorePalette();
		else fontmn2->mapPalette(240, 8, 114, 16);

		fontmn2->showString("choose level:", 32, (canvasH << 1) / 3);
		if (levelNum >= 0) fontmn2->showNumber(levelNum, 208, (canvasH << 1) / 3);
		else fontmn2->showString("bonus", 172, (canvasH << 1) / 3);

		if (option != 0) fontmn2->restorePalette();

		if (controls.release(C_UP)) option ^= 1;

		if (controls.release(C_DOWN)) option ^= 1;

		if (controls.release(C_LEFT)) {

			if (option) levelNum = ((levelNum + 11) % 11) - 1;
			else worldNum = (worldNum + 999) % 1000;

		}

		if (controls.release(C_RIGHT)) {

			if (option) levelNum = ((levelNum + 2) % 11) - 1;
			else worldNum = (worldNum + 1) % 1000;

		}

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			if (newGameDifficulty(M_SINGLE, levelNum, worldNum) == E_QUIT) return E_QUIT;

			video.setPalette(menuPalette);

		}

	}

	return E_NONE;

}


int GameMenu::newGameLevel (GameModeType mode) {

	char* fileName;
	int ret;

	fileName = createString("castle1.j2l");

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


int GameMenu::newGameEpisode (GameModeType mode) {

	const char *options[12] = {"episode 1", "episode 2", "episode 3",
		"episode 4", "episode 5", "episode 6", "episode a", "episode b",
		"episode c", "episode x", "bonus stage", "specific level"};
	bool exists[12];
	char *check;
	SDL_Rect dst;
	int episode, count, worldNum;

	video.setPalette(palette);

	for (count = 0; count < 10; count++) {

		if (count < 6) worldNum = count * 3;
		else if ((count >= 6) && (count < 9)) worldNum = (count + 4) * 3;
		else worldNum = 50;

		check = createFileName(F_LEVEL, 0, worldNum);
		exists[count] = fileExists(check);
		delete[] check;

		if (exists[count]) video.restoreSurfacePalette(episodeScreens[count]);
		else SDL_SetPalette(episodeScreens[count], SDL_LOGPAL, greyPalette, 0, 256);

	}

	if (mode == M_SINGLE) {

		check = createFileName(F_BONUSMAP, 0);
		exists[10] = fileExists(check);
		delete[] check;

	} else exists[10] = false;

	exists[11] = true;

	episode = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		SDL_Delay(T_FRAME);

		clearScreen(0);

		dst.x = canvasW - 144;
		dst.y = (canvasH - 110) >> 1;

		if ((episode < episodes - 1) || (episode < 6)) {

			SDL_BlitSurface(episodeScreens[episode], NULL, canvas, &dst);

		} else if ((episode == 10) && (episodes > 6)) {

			SDL_BlitSurface(episodeScreens[episodes - 1], NULL, canvas, &dst);

		}

		for (count = 0; count < 12; count++) {

			if (count == episode) {

				fontmn2->mapPalette(240, 8, 79, -80);
				drawRect((canvasW >> 3) - 4, (canvasH >> 1) + (count << 4) - 94,
					136, 15, 79);

			} else if (!exists[count])
				fontmn2->mapPalette(240, 8, 94, -16);

			fontmn2->showString(options[count], canvasW >> 3,
				(canvasH >> 1) + (count << 4) - 92);

			if ((count == episode) || (!exists[count]))
				fontmn2->mapPalette(240, 8, 9, 80);

		}

		if (controls.release(C_UP)) episode = (episode + 11) % 12;

		if (controls.release(C_DOWN)) episode = (episode + 1) % 12;

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			if (exists[episode]) {

				if (episode < 10) {

					if (episode < 6) worldNum = episode * 3;
					else if ((episode >= 6) && (episode < 9)) worldNum = (episode + 4) * 3;
					else worldNum = 50;

					if (newGameDifficulty(mode, 0, worldNum) == E_QUIT) return E_QUIT;

				} else if (episode == 10) {

					if (newGameDifficulty(mode, -1, 0) == E_QUIT) return E_QUIT;

				} else {

					if (newGameLevel(mode) == E_QUIT) return E_QUIT;

				}

				video.setPalette(palette);

			}

		}

	}

	return E_NONE;

}


int GameMenu::joinGame () {

	int ret;

	ret = textInput("ip address:", netAddress);

	if (ret < 0) return ret;

	try {

		game = new ClientGame(netAddress);

	} catch (int e) {

		switch (e) {

			case E_N_SOCKET:

				message("SOCKET ERROR");

				break;

			case E_N_ADDRESS:

				message("INVALID ADDRESS");

				break;

			case E_N_CONNECT:

				message("COULD NOT CONNECT");

				break;

			case E_TIMEOUT:

				message("OPERATION TIMED OUT");

				break;

			case E_DATA:

				message("INCORRECT DATA\nRECEIVED");

				break;

			case E_VERSION:

				message("WRONG SERVER VERSION");

				break;

			case E_UNUSED:
			case E_QUIT:

				break;

			default:

				message("COULD COMPLETE CONNECTION");

				break;

		}

		return e;

	}


	// Play the level(s)

	switch (game->play()) {

		case E_QUIT:

			delete game;

			return E_QUIT;

		case E_FILE:

			message("FILE NOT FOUND");

			break;

	}

	delete game;

	return E_NONE;

}


int GameMenu::newGame () {

#if (defined USE_SOCKETS) || (defined USE_SDL_NET)
	const char *newGameOptions[6] = {"new single player game", "new co-op game",
		"new battle", "new team battle", "new race", "join game"};
	int ret;
	int option;

	option = 0;

	while (true) {

		ret = generic(newGameOptions, 6, option);

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

