
/*
 *
 * menu.cpp
 *
 * Created as menu.c on the 23rd of August 2005
 * Renamed menu.cpp on the 3rd of February 2009
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

/*
 * Deals with the loading, running and freeing of the menus.
 *
 */


#include "OpenJazz.h"
#include <string.h>
#include <time.h>


Menu::Menu () {

	File *f;
	time_t currentTime;
	int count, colour;

	try {

		f = new File("openjazz.000", false);

	} catch (int e) {

		throw e;

	}

	screens[14] = f->loadSurface(64, 40);

	delete f;



	try {

		f = new File("menu.000", false);

	} catch (int e) {

		SDL_FreeSurface(screens[14]);

		throw FAILURE;

	}

	f->seek(0, true);

	// Load the main menu graphics
	f->loadPalette(palettes[0]);
	screens[0] = f->loadSurface(320, 200);
	screens[1] = f->loadSurface(320, 200);


	if (f->getSize() > 200000) {

		time(&currentTime);

		// In December, load the Christmas menu graphics
		if (localtime(&currentTime)->tm_mon == 11) {

			SDL_FreeSurface(screens[0]);
			SDL_FreeSurface(screens[1]);
			f->loadPalette(palettes[0]);
			screens[0] = f->loadSurface(320, 200);
			screens[1] = f->loadSurface(320, 200);

		} else {

			f->skipRLE();
			f->skipRLE();
			f->skipRLE();

		}

	}

	SDL_SetColorKey(screens[0], SDL_SRCCOLORKEY, 0);
	SDL_SetColorKey(screens[1], SDL_SRCCOLORKEY, 0);


	// Load the difficulty graphics
	f->loadPalette(palettes[1]);
	screens[2] = f->loadSurface(320, 200);
	SDL_SetColorKey(screens[2], SDL_SRCCOLORKEY, 0);

	// Default difficulty setting
	difficulty = 1;


	// Load the episode pictures (max. 10 episodes + bonus level)

	// Load their palette
	f->loadPalette(palettes[2]);

	// Generate a greyscale mapping
	for (count = 0; count < 256; count++) {

		colour = ((palettes[2][count].r >> 1) +
		          (palettes[2][count].g << 1) +
		          (palettes[2][count].b >> 1)  ) / 8;

		if (colour > 79) colour = 79;

		palettes[3][count].r = palettes[3][count].g =
		palettes[3][count].b = colour;

	}

	episodes = 11;

	for (count = 0; count < 11; count++) {

		screens[count + 3] = f->loadSurface(134, 110);

		if (f->tell() >= f->getSize()) {

			episodes = ++count;

			for (; count < 11; count++)
				screens[count + 3] = createBlankSurface();

		}

	}

	delete f;

	playMusic("menusng.psm");

	return;

}


Menu::~Menu () {

	int count;

	stopMusic();

	for (count = 0; count < 15; count++) SDL_FreeSurface(screens[count]);

	return;

}


int Menu::textInput (char * request, char ** text) {

	char *input;
	SDL_Event event;
	SDL_Rect dst;
	int count, terminate;
	unsigned int cursor;

	input = new char[STRING_LENGTH + 1];
	if (*text) strcpy(input, *text);
	else input[0] = 0;

	cursor = strlen(input);

	while (1) {

		update();


		// Process system events
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

				case SDL_KEYDOWN:

					// Ensure there is space for another character
					if (cursor < STRING_LENGTH) {

						terminate = (input[cursor] == 0);

						if ((event.key.keysym.sym == ' ') ||
							((event.key.keysym.sym >= '0') &&
							(event.key.keysym.sym <= '9')) ||
							((event.key.keysym.sym >= 'A') &&
							(event.key.keysym.sym <= 'Z'))) {

							input[cursor] = event.key.keysym.sym;
							cursor++;
							if (terminate) input[cursor] = 0;

						} else if ((event.key.keysym.sym >= 'a') &&
							(event.key.keysym.sym <= 'z')) {

							input[cursor] = event.key.keysym.sym - 32;
							cursor++;
							if (terminate) input[cursor] = 0;

						}

					}

					if ((event.key.keysym.sym == SDLK_DELETE) &&
						(cursor < strlen(input))) {

						for (count = cursor; count < STRING_LENGTH; count++)
							input[count] = input[count + 1];

					}

					if ((event.key.keysym.sym == SDLK_BACKSPACE) &&
						(cursor > 0)) {

						for (count = cursor - 1; count < STRING_LENGTH; count++)
							input[count] = input[count + 1];

						cursor--;

					}

					// The absence of a break statement is intentional

				case SDL_KEYUP:

					for (count = 0; count < CONTROLS; count++)
						if (event.key.keysym.sym == keys[count].key)
							keys[count].state = event.key.state;

					break;

				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:

					for (count = 0; count < CONTROLS; count++)
						if (event.jbutton.button == buttons[count].button)
							buttons[count].state = event.jbutton.state;

					break;

				case SDL_JOYAXISMOTION:

					for (count = 0; count < CONTROLS; count++)
						if (event.jaxis.axis == axes[count].axis) {

							if (!axes[count].direction &&
								(event.jaxis.value < -16384))
								axes[count].state = SDL_PRESSED;

							else if (axes[count].direction &&
								(event.jaxis.value > 16384))
								axes[count].state = SDL_PRESSED;

							else axes[count].state = SDL_RELEASED;

						}

					break;

#ifndef FULLSCREEN_ONLY
				case SDL_VIDEORESIZE:

					screenW = event.resize.w;
					screenH = event.resize.h;
					screen = SDL_SetVideoMode(screenW, screenH, 8,
						SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE |
						SDL_HWPALETTE);

					// The absence of a break statement is intentional

				case SDL_VIDEOEXPOSE:

					SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
					SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

					break;
#endif

				case SDL_QUIT:

					delete[] input;

					return QUIT;

			}

		}

		// Apply controls to universal control tracking
		for (count = 0; count < CONTROLS; count++) {

			if ((keys[count].state == SDL_PRESSED) ||
				(buttons[count].state == SDL_PRESSED) || 
				(axes[count].state == SDL_PRESSED)      ) {

				if (controls[count].time < (int)SDL_GetTicks())
					controls[count].state = SDL_PRESSED;

			} else {

				controls[count].time = 0;
				controls[count].state = SDL_RELEASED;

			}

		}


		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			delete[] input;

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 15);

		// Draw the prompt
		fontmn2->showString(request, 32, (screenH / 2) - 16);

		// Draw the section of the text before the cursor
		fontmn2->scalePalette(F2, (-240 * 2) + 114);
		terminate = input[cursor];
		input[cursor] = 0;
		dst.x = fontmn2->showString(input, 40, screenH / 2);

		// Draw the cursor
		dst.w = 8;
		dst.h = 2;
		dst.y = (screenH / 2) + 10;
		SDL_FillRect(screen, &dst, 79);

		// Draw the section of text after the cursor
		input[cursor] = terminate;
		fontmn2->showString(input + cursor, dst.x, screenH / 2);
		fontmn2->restorePalette();


		if (controls[C_LEFT].state == SDL_PRESSED) {

			releaseControl(C_LEFT);

			if (cursor > 0) cursor--;

		}

		if (controls[C_RIGHT].state == SDL_PRESSED) {

			releaseControl(C_RIGHT);

			if (cursor < strlen(input)) cursor++;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			*text = input;

			return SUCCESS;

		}

	}

	delete[] input;

	return SUCCESS;

}


int Menu::newGameDifficulty () {

	Scene * sceneInst;
	char *options[4] = {"EASY", "MEDIUM", "HARD", "TURBO"};
	SDL_Rect src, dst;
	int count;
	bool checkpoint;

	usePalette(palettes[1]);

	checkpoint = false;

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 0);

		for (count = 0; count < 4; count++) {

			if (count == difficulty)
				fontmn2->scalePalette(F2, (-240 * 2) + 114);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 32);

			if (count == difficulty) fontmn2->restorePalette();

		}

		src.x = (difficulty & 1) * 160;
		src.y = (difficulty & 2) * 50;
		src.w = 160;
		src.h = 100;
		dst.x = (screenW >> 1) - 40;
		dst.y = (screenH >> 1) - 50;
		SDL_BlitSurface(screens[2], &src, screen, &dst);

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);
			difficulty = (difficulty + 3) % 4;

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);
			difficulty = (difficulty + 1) % 4;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			stopMusic();

			// Create the player
			nPlayers = 1;
			players = new Player[nPlayers];
			players[0].setName(localPlayerName);


			// Load the level

			try {

				levelInst = new Level(nextLevel, difficulty, false);

			} catch (int e) {

				delete[] players;

				return FAILURE;

			}

			// Play the level(s)
			while (1) {

				switch (levelInst->run()) {

					case QUIT: // Quit

						delete levelInst;
						delete[] players;

						return QUIT;

					case SUCCESS: // Quit game

						delete levelInst;
						delete[] players;

						playMusic("menusng.psm");

						return SUCCESS;

					case FAILURE: // Error

						delete levelInst;
						delete[] players;

						return FAILURE;

					case WON: // Completed level

						// If there is no next level, load and run the cutscene
						if (!strcmp(nextLevel, "endepis")) {

							delete[] players;

							sceneInst = levelInst->createScene();

							delete levelInst;

							sceneInst->run();

							delete sceneInst;

							return SUCCESS;

						}

						// Do not use old level's checkpoint coordinates
						checkpoint = false;

						break;

					case LOST: // Lost level

						if (!players[0].getLives()) {

							delete levelInst;
							delete[] players;

							return SUCCESS; // Not really a success...

						}

						// Set to load the same level again
						delete[] nextLevel;
						nextLevel = levelInst->currentFile;

						// Have something to free in the levelInst destructor
						levelInst->currentFile = new char[1];

						// Use checkpoint coordinates
						checkpoint = true;

						break;

				}

				// Use the menu palette for the level loading screen
				usePalette(palettes[1]);

				// Unload the previous level
				delete levelInst;

				// Load the level

				try {

					levelInst = new Level(nextLevel, difficulty, checkpoint);

				} catch (int e) {

					delete[] players;

					return FAILURE;

				}

			}

		}

	}

	return SUCCESS;

}


int Menu::newGameLevel () {

	int option, worldNum, levelNum;

	worldNum = levelNum = option = 0;

	usePalette(palettes[1]);

	while (1) {

		if (loop()) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 15);

		if (option == 0) fontmn2->scalePalette(F2, (-240 * 2) + 114);
		fontmn2->showString("CHOOSE WORLD:", 32, screenH / 3);
		fontmn2->showNumber(worldNum, 208, screenH / 3);

		if (option == 0) fontmn2->restorePalette();
		else fontmn2->scalePalette(F2, (-240 * 2) + 114);

		fontmn2->showString("CHOOSE LEVEL:", 32, (screenH << 1) / 3);
		fontmn2->showNumber(levelNum, 208, (screenH << 1) / 3);

		if (option != 0) fontmn2->restorePalette();

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);

			option ^= 1;

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);

			option ^= 1;

		}

		if (controls[C_LEFT].state == SDL_PRESSED) {

			releaseControl(C_LEFT);

			if (option) levelNum = (levelNum + 9) % 10;
			else worldNum = (worldNum + 999) % 1000;

		}

		if (controls[C_RIGHT].state == SDL_PRESSED) {

			releaseControl(C_RIGHT);

			if (option) levelNum = (levelNum + 1) % 10;
			else worldNum = (worldNum + 1) % 1000;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			nextLevel = new char[11];
			sprintf(nextLevel, "level%1i.%03i", levelNum, worldNum);

			if (newGameDifficulty() == QUIT) {

				delete[] nextLevel;

				return QUIT;

			}

			delete[] nextLevel;

			usePalette(palettes[1]);

		}

	}

	return SUCCESS;

}


int Menu::newGame () {

	char *options[12] = {"EPISODE 1", "EPISODE 2", "EPISODE 3", "EPISODE 4",
		"EPISODE 5", "EPISODE 6", "EPISODE A", "EPISODE B", "EPISODE C",
		"EPISODE X", "BONUS STAGE", "SPECIFIC LEVEL"};
	int exists[12];
	char check[11];
	SDL_Rect dst;
	int episode, count, worldNum;

	usePalette(palettes[2]);

	for (count = 0; count < 10; count++) {

		if (count < 6) worldNum = count * 3;
		else if ((count >= 6) && (count < 9)) worldNum = (count + 4) * 3;
		else worldNum = 50;

		sprintf(check, "level0.%03i", worldNum);

		exists[count] = fileExists(check);

		if (exists[count]) restorePalette(screens[count + 3]);
		else
			SDL_SetPalette(screens[count + 3], SDL_LOGPAL, palettes[3], 0, 256);

	}

	exists[10] = 0;
	exists[11] = 1;

	episode = 0;

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 0);

		if ((episode < episodes - 1) || (episode < 6)) {

			dst.x = screenW - 150;
			dst.y = (screenH - 110) >> 1;
			SDL_BlitSurface(screens[episode + 3], NULL, screen, &dst);

		} else if ((episode == 10) && (episodes > 6)) {

			dst.x = screenW - 160;
			dst.y = (screenH - 110) >> 1;
			SDL_BlitSurface(screens[episodes + 2], NULL, screen, &dst);

		} else if (episode == 11) {

			// To do

		}

		for (count = 0; count < 12; count++) {

			if (count == episode) {

				fontmn2->scalePalette(-F10, (-240 * -10) + 79);
				dst.x = (screenW >> 3) - 4;
				dst.y = (screenH >> 1) + (count << 4) - 94;
				dst.w = 136;
				dst.h = 15;
				SDL_FillRect(screen, &dst, 79);

			} else if (!exists[count])
				fontmn2->scalePalette(-F2, (-240 * -2) + 94);

			fontmn2->showString(options[count], screenW >> 3,
				(screenH >> 1) + (count << 4) - 92);

			if ((count == episode) || (!exists[count]))
				fontmn2->scalePalette(F10, (-240 * 10) + 9);

		}

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);

			episode = (episode + 11) % 12;

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);

			episode = (episode + 1) % 12;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			if (exists[episode]) {

				if (episode < 10) {

					if (episode < 6) worldNum = episode * 3;
					else if ((episode >= 6) && (episode < 9))
						worldNum = (episode + 4) * 3;
					else worldNum = 50;

					nextLevel = new char[11];
					sprintf(nextLevel, "level0.%03i", worldNum);

					if (newGameDifficulty() == QUIT) {

						delete[] nextLevel;

						return QUIT;

					}

					delete[] nextLevel;

				} else if (episode == 10) {

					// To do

				} else {

					if (newGameLevel() == QUIT) return QUIT;

				}

				usePalette(palettes[2]);

			}

		}

	}

	return SUCCESS;

}


int Menu::loadGame () {

	// To do

	return SUCCESS;

}


int Menu::setupCharacter () {

	char *options[1] = {"NAME"};
	char *result;
	int option, count;

	option = 0;

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 0);

		for (count = 0; count < 1; count++) {

			if (count == option) fontmn2->scalePalette(F2, (-240 * 2) + 114);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 32);

			if (count == option) fontmn2->restorePalette();

		}

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);

			option = (option + 1) % 1;

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);

			option = (option + 1) % 1;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			switch (option) {

				case 0:

					result = localPlayerName;
					textInput("CHARACTER NAME:", &result);

					if (result != localPlayerName) {

						delete[] localPlayerName;
						localPlayerName = result;

					}

					break;

			}

		}

	}

	return SUCCESS;

}


int Menu::setupKeyboard () {

	char *options[7] = {"UP", "DOWN", "LEFT", "RIGHT", "JUMP", "FIRE",
		"WEAPON"};
	SDL_Event event;
	int progress, count, used;

	progress = 0;

	while (1) {

		update();


		// Process system events
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

				case SDL_KEYDOWN:
				case SDL_KEYUP:

					used = 0;

					for (count = 0; count < CONTROLS; count++)
						if (event.key.keysym.sym == keys[count].key) {

							keys[count].state = event.key.state;
							if (count != progress) used = 1;

						}

					if (!used) {

						keys[progress].key = event.key.keysym.sym;
						progress++;

						if (progress == 7) return SUCCESS;

					}

					break;

#ifndef FULLSCREEN_ONLY
				case SDL_VIDEORESIZE:

					screenW = event.resize.w;
					screenH = event.resize.h;
					screen = SDL_SetVideoMode(screenW, screenH, 8,
						SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE |
						SDL_HWPALETTE);

					// The absence of a break statement is intentional

				case SDL_VIDEOEXPOSE:

					SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
					SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

					break;
#endif

				case SDL_QUIT:

					return QUIT;

			}

		}

		// Apply controls to universal control tracking
		for (count = 0; count < CONTROLS; count++) {

			if ((keys[count].state == SDL_PRESSED) ||
				(buttons[count].state == SDL_PRESSED) || 
				(axes[count].state == SDL_PRESSED)      ) {

				if (controls[count].time < (int)SDL_GetTicks())
					controls[count].state = SDL_PRESSED;

			} else {

				controls[count].time = 0;
				controls[count].state = SDL_RELEASED;

			}

		}


		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 0);

		for (count = 0; count < 7; count++) {

			if (count < progress)
				fontmn2->showString("OKAY", (screenW >> 2) + 176,
					(screenH >> 1) + (count << 4) - 32);

			else if (count == progress)
				fontmn2->scalePalette(F2, (-240 * 2) + 114);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 32);

			if (count == progress) {

				fontmn2->showString("PRESS KEY", (screenW >> 2) + 112,
					(screenH >> 1) + (count << 4) - 32);

				fontmn2->restorePalette();

			}

		}

	}

	return SUCCESS;

}


int Menu::setupJoystick () {

	char *options[7] = {"UP", "DOWN", "LEFT", "RIGHT", "JUMP", "FIRE",
		"WEAPON"};
	SDL_Event event;
	int progress, count, used;

	progress = 0;

	while (1) {

		update();


		// Process system events
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

				case SDL_KEYDOWN:
				case SDL_KEYUP:

					for (count = 0; count < CONTROLS; count++)
						if (event.key.keysym.sym == keys[count].key)
							keys[count].state = event.key.state;

					break;

				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:

					used = 0;

					for (count = 0; count < CONTROLS; count++)
						if (event.jbutton.button == buttons[count].button) {

							buttons[count].state = event.jbutton.state;
							if (count != progress) used = 1;

						}

					if (!used) {

						buttons[progress].button = event.jbutton.button;
						progress++;

						if (progress == 7) return SUCCESS;

					}

					break;

				case SDL_JOYAXISMOTION:

					used = 0;

					for (count = 0; count < CONTROLS; count++)
						if (event.jaxis.axis == axes[count].axis) {

							if (!axes[count].direction &&
								(event.jaxis.value < -16384)) {

								axes[count].state = SDL_PRESSED;
								if (count != progress) used = 1;

							} else if (axes[count].direction &&
								(event.jaxis.value > 16384)) {

								axes[count].state = SDL_PRESSED;
								if (count != progress) used = 1;

							} else axes[count].state = SDL_RELEASED;

						}

						if (!used && ((event.jaxis.value < -16384) ||
							(event.jaxis.value > 16384))) {

							axes[progress].axis = event.jaxis.axis;
							if (event.jaxis.value < -16384)
								axes[progress].direction = 0;
							else axes[progress].direction = 1;
							progress++;

							if (progress == 7) return SUCCESS;

						}

						break;

#ifndef FULLSCREEN_ONLY
					case SDL_VIDEORESIZE:

						screenW = event.resize.w;
						screenH = event.resize.h;
						screen = SDL_SetVideoMode(screenW, screenH, 8,
							SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE |
							SDL_HWPALETTE);

						// The absence of a break statement is intentional

					case SDL_VIDEOEXPOSE:

						SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0,
							256);
						SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0,
							256);

						break;
#endif

					case SDL_QUIT:

						return QUIT;

				}

		}

		// Apply controls to universal control tracking
		for (count = 0; count < CONTROLS; count++) {

			if ((keys[count].state == SDL_PRESSED) ||
				(buttons[count].state == SDL_PRESSED) || 
				(axes[count].state == SDL_PRESSED)      ) {

				if (controls[count].time < (int)SDL_GetTicks())
					controls[count].state = SDL_PRESSED;

			} else {

				controls[count].time = 0;
				controls[count].state = SDL_RELEASED;

			}

		}


		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 0);

		for (count = 0; count < 7; count++) {

			if (count < progress)
				fontmn2->showString("OKAY", (screenW >> 2) + 176,
					(screenH >> 1) + (count << 4) - 32);

			else if (count == progress)
				fontmn2->scalePalette(F2, (-240 * 2) + 114);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 32);

			if (count == progress) {

				fontmn2->showString("PRESS CONTROL", (screenW >> 2) + 112,
					(screenH >> 1) + (count << 4) - 32);

				fontmn2->restorePalette();

			}

		}

	}

	return SUCCESS;

}


int Menu::setupResolution () {

#ifndef FULLSCREEN_ONLY

	int widthOptions[] = {320, 400, 512, 640, 720, 768, 800, 960, 1024, 1152,
		1280, 1440, 1600, 1920};
	int heightOptions[] = {200, 240, 300, 384, 400, 480, 576, 600, 720, 768,
		800, 864, 900, 960, 1024, 1080, 1200};
	SDL_Rect **resolutions;
	SDL_Rect dst;
	int dimension, count, maxW, maxH;

	dimension = 0;

	if (fullscreen)
		resolutions = SDL_ListModes(NULL,
			SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);
	else 
		resolutions = SDL_ListModes(NULL,
			SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);

	if (resolutions == (SDL_Rect **)(-1)) {

		maxW = 1920;
		maxH = 1200;

	} else {

		maxW = 320;
		maxH = 200;

		for (count = 0; resolutions[count] != NULL; count++) {

			if (resolutions[count]->w > maxW) maxW = resolutions[count]->w;
			if (resolutions[count]->h > maxH) maxH = resolutions[count]->h;

		}

	}

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			return SUCCESS;

		}

		count = 0;

		SDL_FillRect(screen, NULL, 0);


		// Show screen corners
		dst.w = 32;
		dst.h = 32;
		dst.x = 0;
		dst.y = 0;
		SDL_FillRect(screen, &dst, 79);
		dst.x = screenW - 32;
		SDL_FillRect(screen, &dst, 79);
		dst.y = screenH - 32;
		SDL_FillRect(screen, &dst, 79);
		dst.x = 0;
		SDL_FillRect(screen, &dst, 79);


		fontmn2->showString("X", (screenW >> 2) + 40, screenH >> 1);

		if (dimension == 0) fontmn2->scalePalette(F2, (-240 * 2) + 114);

		fontmn2->showNumber(screenW, (screenW >> 2) + 32, screenH >> 1);

		if (dimension == 0) fontmn2->restorePalette();
		else fontmn2->scalePalette(F2, (-240 * 2) + 114);

		fontmn2->showNumber(screenH, (screenW >> 2) + 104, screenH >> 1);

		if (dimension != 0) fontmn2->restorePalette();


		if (controls[C_LEFT].state == SDL_PRESSED) {

			releaseControl(C_LEFT);

			dimension = !dimension;

		}

		if (controls[C_RIGHT].state == SDL_PRESSED) {

			releaseControl(C_RIGHT);

			dimension = !dimension;

		}

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);

			if ((dimension == 0) && (screenW < maxW)) {

				while (screenW >= widthOptions[count]) count++;

				screenW = widthOptions[count];

			}

			if ((dimension == 1) && (screenH < maxH)) {

				while (screenH >= heightOptions[count]) count++;

				screenH = heightOptions[count];

			}

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);

			if ((dimension == 0) && (screenW > 320)) {

				count = 13;

				while (screenW <= widthOptions[count]) count--;

				screenW = widthOptions[count];
				count = -1;

			}

			if ((dimension == 1) && (screenH > 200)) {

				count = 16;

				while (screenH <= heightOptions[count]) count--;

				screenH = heightOptions[count];
				count = -1;

			}

		}

		// Check for a resolution change
		if (count) {

			if (fullscreen)
				screen = SDL_SetVideoMode(screenW, screenH, 8,
					SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE |
						SDL_HWPALETTE);

			else
				screen = SDL_SetVideoMode(screenW, screenH, 8,
					SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE |
						SDL_HWPALETTE);

		}

	}

#endif

	return SUCCESS;

}


int Menu::setup () {

	char *options[4] = {"CHARACTER", "KEYBOARD", "JOYSTICK", "RESOLUTION"};
	int option, count;

	usePalette(palettes[1]);

	option = 0;

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 0);

		for (count = 0; count < 4; count++) {

			if (count == option) fontmn2->scalePalette(F2, (-240 * 2) + 114);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 32);

			if (count == option) fontmn2->restorePalette();

		}

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);

			option = (option + 3) % 4;

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);

			option = (option + 1) % 4;

		}

		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			switch (option) {

				case 0:

					setupCharacter();

					break;

				case 1:

					setupKeyboard();

					break;

				case 2:

					setupJoystick();

					break;

				case 3:

					setupResolution();

					break;

			}

		}

	}

	return SUCCESS;

}


int Menu::run () {

	Scene *sceneInst;
	int option;
	SDL_Rect src, dst;

	option = 0;

	usePalette(palettes[0]);

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		SDL_FillRect(screen, NULL, 28);

		dst.x = (screenW >> 2) - 72;
		dst.y = screenH - (screenH >> 2);
		SDL_BlitSurface(screens[14], NULL, screen, &dst);

		dst.x = (screenW - 320) >> 1;
		dst.y = (screenH - 200) >> 1;
		SDL_BlitSurface(screens[0], NULL, screen, &dst);

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

		dst.x = ((screenW - 320) >> 1) + src.x;
		dst.y = ((screenH - 200) >> 1) + src.y;
		SDL_BlitSurface(screens[1], &src, screen, &dst);

		if (controls[C_UP].state == SDL_PRESSED) {

			releaseControl(C_UP);

			option = (option + 5) % 6;

		}

		if (controls[C_DOWN].state == SDL_PRESSED) {

			releaseControl(C_DOWN);

			option = (option + 1) % 6;

		}


		if (controls[C_ENTER].state == SDL_PRESSED) {

			releaseControl(C_ENTER);

			switch(option) {

				case 0: // New game

					if (newGame() == QUIT) return QUIT;

					break;

				case 1: // Load game

					if (loadGame() == QUIT) return QUIT;

					break;

				case 2: // Instructions

					sceneInst = new Scene("instruct.0sc");

					if (sceneInst->run() == QUIT) {

						delete sceneInst;

						return QUIT;

					}

					delete sceneInst;

					break;

				case 3: // Setup options

					if (setup() == QUIT) return QUIT;

					break;

				case 4: // Order info

					sceneInst = new Scene("order.0sc");

					if (sceneInst->run() == QUIT) {

						delete sceneInst;

						return QUIT;

					}

					delete sceneInst;

					break;

				case 5: // Exit

					return SUCCESS;

			}

			usePalette(palettes[0]);

		}

	}

	return SUCCESS;

}


void Menu::setNextLevel (char *fn) {

	delete[] nextLevel;
	nextLevel = fn;

	return;

}


