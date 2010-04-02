
/*
 *
 * mainmenu.cpp
 *
 * 19th July 2009: Created menumain.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menumain.cpp to mainmenu.cpp
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
 * Deals with the running of the main menu and its generic sub-menus.
 *
 */


#include "menu.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "level/level.h"
#include "player/player.h"
#include "scene/scene.h"


int Menu::main () {

#if (defined USE_SOCKETS) || (defined USE_SDL_NET)
	const char *newGameOptions[6] = {"new single player game", "new co-op game",
		"new battle", "new team battle", "new race", "join game"};
	int ret;
#endif
	Scene *scene;
	SDL_Rect src, dst;
	int option, suboption;
	unsigned int idleTime;

	option = suboption = 0;

	usePalette(palettes[0]);

	// Demo timeout
	idleTime = globalTicks + T_DEMO;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) option = 5;

		if (controls.release(C_UP)) option = (option + 5) % 6;

		if (controls.release(C_DOWN)) option = (option + 1) % 6;

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			switch(option) {

				case 0: // New game

#if (defined USE_SOCKETS) || (defined USE_SDL_NET)
					while (true) {

						ret = generic(newGameOptions, 6, &suboption);

						if (ret == E_QUIT) return E_QUIT;
						if (ret < 0) break;

						if (suboption == 5) {

							if (joinGame() == E_QUIT) return E_QUIT;

						} else {

							if (newGameEpisode(GameModeType(suboption)) == E_QUIT) return E_QUIT;

						}

					}
#else
					if (newGameEpisode(suboption) == E_QUIT) return E_QUIT;
#endif

					break;

				case 1: // Load game

					if (loadGame() == E_QUIT) return E_QUIT;

					break;

				case 2: // Instructions

					try {

						scene = new Scene("instruct.0sc");

					} catch (int e) {

						message("COULD NOT LOAD INSTRUCTIONS");

						break;

					}

					if (scene->play() == E_QUIT) {

						delete scene;

						return E_QUIT;

					}

					delete scene;

					break;

				case 3: // Setup options

					if (setup() == E_QUIT) return E_QUIT;

					break;

				case 4: // Order info

					try {

						scene = new Scene("order.0sc");

					} catch (int e) {

						message("COULD NOT LOAD ORDER INFO");

						break;

					}

					if (scene->play() == E_QUIT) {

						delete scene;

						return E_QUIT;

					}

					delete scene;

					break;

				case 5: // Exit

					return E_NONE;

			}

			// Restore the main menu palette
			usePalette(palettes[0]);

			// New demo timeout
			idleTime = globalTicks + T_DEMO;

		}


		if (idleTime <= globalTicks) {

			game = NULL;


			// Create the player
			nPlayers = 1;
			localPlayer = players = new Player[1];
			localPlayer->init(characterName, NULL, 0);


			// Load the macro

			try {

				level = new DemoLevel(F_MACRO);

			} catch (int e) {

				delete[] players;
				localPlayer = NULL;

				break;

			}

			// Play the level
			if (level->play() == E_QUIT) {

				delete level;
				delete[] players;

				return E_QUIT;

			}

			delete level;
			delete[] players;
			localPlayer = NULL;

			playMusic("menusng.psm");

			// Restore the main menu palette
			usePalette(palettes[0]);

			idleTime = globalTicks + T_DEMO;

		}

		SDL_Delay(T_FRAME);

		clearScreen(28);

		dst.x = (canvasW >> 2) - 72;
		dst.y = canvasH - (canvasH >> 2);
		SDL_BlitSurface(screens[14], NULL, canvas, &dst);

		dst.x = (canvasW - SW) >> 1;
		dst.y = (canvasH - SH) >> 1;
		SDL_BlitSurface(screens[0], NULL, canvas, &dst);

		switch (option) {

			case 0:

				src.x = 92;
				src.y = 35;
				src.w = 136;
				src.h = 22;

				break;

			case 1:

				src.x = 92;
				src.y = 57;
				src.w = 140;
				src.h = 22;

				break;

			case 2:

				src.x = 88;
				src.y = 83;
				src.w = 144;
				src.h = 22;

				break;

			case 3:

				src.x = 86;
				src.y = 109;
				src.w = 150;
				src.h = 23;

				break;

			case 4:

				src.x = 82;
				src.y = 137;
				src.w = 156;
				src.h = 26;

				break;

			case 5:

				src.x = 78;
				src.y = 166;
				src.w = 166;
				src.h = 29;

				break;

		}

		dst.x = ((canvasW - SW) >> 1) + src.x;
		dst.y = ((canvasH - SH) >> 1) + src.y;
		SDL_BlitSurface(screens[1], &src, canvas, &dst);

	}

	return E_NONE;

}


