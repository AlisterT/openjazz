
/**
 *
 * @file mainmenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd of August 2005: Created menu.c
 * - 3rd of February 2009: Renamed menu.c to menu.cpp
 * - 19th July 2009: Created menumain.cpp from parts of menu.cpp
 * - 26th July 2009: Renamed menumain.cpp to mainmenu.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the running of the main menu and its generic sub-menus.
 *
 */


#include "menu.h"
#include "plasma.h"

#include "game/game.h"
#include "io/controls.h"
#include "io/gfx/video.h"
#include "io/gfx/font.h"
#include "io/sound.h"
#include "jj1/scene/jj1scene.h"
#include "loop.h"
#include "util.h"
#include "io/log.h"
#include "logo.h"
#include "version.h"

#include <miniz.h>
#include <time.h>


/**
 * Create the main menu.
 */
MainMenu::MainMenu () {

	File *file;
	time_t currentTime;

	// Load the OpenJazz logo

	unsigned char* pixels = new unsigned char[oj_logo.size];
	size_t res = tinfl_decompress_mem_to_mem(pixels, oj_logo.size, oj_logo.data,
		oj_logo.compressed_size, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF|TINFL_FLAG_PARSE_ZLIB_HEADER);
	if (res == TINFL_DECOMPRESS_MEM_TO_MEM_FAILED || res != oj_logo.size) {

		LOG_WARN("Could not uncompress logo (expected: %d, actual: %zu).", oj_logo.size, res);
		logo = nullptr;

	} else {

		logo = createSurface(pixels, oj_logo.width, oj_logo.height);

	}
	delete[] pixels;

	// Load the menu graphics

	try {

		file = new File("MENU.000", PATH_TYPE_GAME);

	} catch (int e) {

		if (logo) SDL_FreeSurface(logo);

		throw;

	}

	// only available in Holiday Hare 94/95
	if (file->getSize() > 200000) {

		time(&currentTime);

		// In December, load the Christmas menu graphics
		if (localtime(&currentTime)->tm_mon == 11) {

			LOG_TRACE("Loading christmas menu theme.");

			file->skipRLE();
			file->skipRLE();
			file->skipRLE();

			file->loadPalette(palette);
			background = file->loadSurface(SW, SH);
			highlight = file->loadSurface(SW, SH);

		} else {

			file->loadPalette(palette);
			background = file->loadSurface(SW, SH);
			highlight = file->loadSurface(SW, SH);

			file->skipRLE();
			file->skipRLE();
			file->skipRLE();

		}

	} else {

		file->loadPalette(palette);
		background = file->loadSurface(SW, SH);
		highlight = file->loadSurface(SW, SH);

	}

	enableColorKey(background, 0);
	enableColorKey(highlight, 0);
	if (logo) enableColorKey(logo, 28);

	gameMenu = new GameMenu(file);

	delete file;

}


/**
 * Delete the main menu.
 */
MainMenu::~MainMenu () {

	SDL_FreeSurface(background);
	SDL_FreeSurface(highlight);
	if (logo) SDL_FreeSurface(logo);

	delete gameMenu;

}


/**
 * Process a main menu selection.
 *
 * @param option Chosen menu option
 *
 * @return Error code
 */
int MainMenu::select (int option) {

	JJ1Scene *scene;
	SetupMenu setupMenu;

	playConfirmSound();

	switch (option) {

		case 0: // New game

			if (gameMenu->newGame() == E_QUIT) return E_QUIT;

			break;

		case 1: // Load game

			if (gameMenu->loadGame() == E_QUIT) return E_QUIT;

			break;

		case 2: // Instructions

			try {

				scene = new JJ1Scene("INSTRUCT.0SC");

			} catch (int e) {

				if (message("COULD NOT LOAD INSTRUCTIONS") == E_QUIT) return E_QUIT;

				break;

			}

			if (scene->play() == E_QUIT) {

				delete scene;

				return E_QUIT;

			}

			delete scene;

			break;

		case 3: // Setup options

			if (setupMenu.setupMain() == E_QUIT) return E_QUIT;

			break;

		case 4: // Order info

			try {

				scene = new JJ1Scene("ORDER.0SC");

			} catch (int e) {

				if (message("COULD NOT LOAD ORDER INFO") == E_QUIT) return E_QUIT;

				break;

			}

			if (scene->play() == E_QUIT) {

				delete scene;

				return E_QUIT;

			}

			delete scene;

			break;

		case 5: // Exit

			return E_RETURN;

	}

	// Restore the main menu palette
	video.setPalette(palette);

	return E_NONE;

}


/**
 * Run the main menu.
 *
 * @return Error code
 */
int MainMenu::main () {

	SDL_Rect options[6] = {
		{92, 35, 136, 22},
		{92, 57, 140, 22},
		{88, 83, 144, 22},
		{86, 109, 150, 23},
		{82, 137, 156, 26},
		{78, 166, 166, 29}};
	int macroType[4];
	File* file;
	char* fileName;
	Plasma plasma;
	SDL_Rect dst;
	int option, macro, x, y, ret;
	unsigned int idleTime;

	option = 0;

	video.setPalette(palette);

	playMusic("MENUSNG.PSM");


	// Demo timeout
	idleTime = globalTicks + T_DEMO;

	// Check for demo macros

	fileName = createString("MACRO.1");

	for (macro = 0; macro < 4; macro++)
	{

		try {

			file = new File(fileName, PATH_TYPE_GAME);
			macroType[macro] = file->loadChar();
			delete file;

		} catch (int e) {

			macroType[macro] = -1;

		}

		fileName[6]++;

	}

	macro = 3;

	delete[] fileName;


	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) option = 5;

		if (controls.release(C_UP)) option = (option + 5) % 6;

		if (controls.release(C_DOWN)) option = (option + 1) % 6;

		if (controls.release(C_ENTER)) {

			ret = select(option);

			if (ret < 0) return ret;

			// New demo timeout
			idleTime = globalTicks + T_DEMO;

		}

		if (controls.getCursor(x, y)) {

			int count;

			x -= (canvasW - SW) >> 1;
			y -= (canvasH - SH) >> 1;

			for (count = 0; count < 6; count++) {

				if ((x >= options[count].x) &&
					(x < options[count].x + options[count].w) &&
					(y >= options[count].y) &&
					(y < options[count].y + options[count].h)) {

					option = count;

					if (controls.wasCursorReleased()) {

						ret = select(option);

						if (ret < 0) return ret;

					}

					// New demo timeout
					idleTime = globalTicks + T_DEMO;

					break;

				}
			}

		}


		if (idleTime <= globalTicks) {

			Game* game = NULL;

			try {

				game = new LocalGame("", 0);

			} catch (int e) {

				// Do nothing

			}

			if (game) {

				// Load the macro

				x = macro;
				macro = (macro + 1) & 3;

				while ((macroType[macro] != 0xFF) && (macro != x))
					macro = (macro + 1) & 3;

				if (macro != x) {

					fileName = createString("MACRO.1");
					fileName[6] += macro;

					if (game->playLevel(fileName) == E_QUIT) {

						delete[] fileName;
						delete game;

						return E_QUIT;

					}

					delete[] fileName;

				}


				delete game;

				playMusic("MENUSNG.PSM");

				// Restore the main menu palette
				video.setPalette(palette);

			}

			idleTime = globalTicks + T_DEMO;

		}

		SDL_Delay(T_MENU_FRAME);


		//as long as we're drawing plasma, we don't need to clear the screen.
		//video.clearScreen(28);

		plasma.draw();

		// draw logo and version string

		if (logo) {
			dst.x = (canvasW >> 2) - 72;
			dst.y = canvasH - (canvasH >> 2);
			SDL_BlitSurface(logo, NULL, canvas, &dst);
		}

		panelBigFont->mapPalette(0, MAX_PALETTE_COLORS, 8, 8);
		panelBigFont->showString("OpenJazz", 1, canvasH - 9);
		panelBigFont->showString(oj_version, 90, canvasH - 9);
		panelBigFont->restorePalette();

		dst.x = (canvasW - SW) >> 1;
		dst.y = (canvasH - SH) >> 1;
		SDL_BlitSurface(background, NULL, canvas, &dst);

		dst.x = ((canvasW - SW) >> 1) + options[option].x;
		dst.y = ((canvasH - SH) >> 1) + options[option].y;
		SDL_BlitSurface(highlight, options + option, canvas, &dst);

	}

	return E_NONE;

}


/**
 * Skip the main menu. Directly loads game
 *
 * @return Error code
 */
int MainMenu::skip (int levelNum, int worldNum) {

	if (gameMenu->loadGame(levelNum, worldNum) == E_QUIT) return E_QUIT;

	return E_RETURN;

}
