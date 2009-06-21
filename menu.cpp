
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


#include "font.h"
#include "game.h"
#include "level.h"
#include "menu.h"
#include "palette.h"
#include "sound.h"
#include <string.h>
#include <time.h>


Menu::Menu () {

	File *file;
	unsigned char *pixels;
	time_t currentTime;
	int count, col;


	// Load the OpenJazz logo

	try {

		file = new File(LOGO_FILE, false);

	} catch (int e) {

		throw e;

	}

	screens[14] = file->loadSurface(64, 40);

	delete file;


	// Load the menu graphics

	try {

		file = new File(F_MENU, false);

	} catch (int e) {

		SDL_FreeSurface(screens[14]);

		throw e;

	}

	file->seek(0, true);

	// Load the main menu graphics
	file->loadPalette(palettes[0]);
	screens[0] = file->loadSurface(320, 200);
	screens[1] = file->loadSurface(320, 200);


	if (file->getSize() > 200000) {

		time(&currentTime);

		// In December, load the Christmas menu graphics
		if (localtime(&currentTime)->tm_mon == 11) {

			SDL_FreeSurface(screens[0]);
			SDL_FreeSurface(screens[1]);
			file->loadPalette(palettes[0]);
			screens[0] = file->loadSurface(320, 200);
			screens[1] = file->loadSurface(320, 200);

		} else {

			file->skipRLE();
			file->skipRLE();
			file->skipRLE();

		}

	}

	SDL_SetColorKey(screens[0], SDL_SRCCOLORKEY, 0);
	SDL_SetColorKey(screens[1], SDL_SRCCOLORKEY, 0);


	// Load the difficulty graphics
	file->loadPalette(palettes[1]);
	screens[2] = file->loadSurface(320, 200);
	SDL_SetColorKey(screens[2], SDL_SRCCOLORKEY, 0);

	// Default difficulty setting
	difficulty = 1;


	// Load the episode pictures (max. 10 episodes + bonus level)

	// Load their palette
	file->loadPalette(palettes[2]);

	// Generate a greyscale mapping
	for (count = 0; count < 256; count++) {

		col = ((palettes[2][count].r >> 1) + (palettes[2][count].g << 1) +
			(palettes[2][count].b >> 1)) >> 3;

		if (col > 79) col = 79;

		palettes[3][count].r = palettes[3][count].g = palettes[3][count].b =
			col;

	}

	episodes = 11;

	for (count = 0; count < 11; count++) {

		screens[count + 3] = file->loadSurface(134, 110);

		if (file->tell() >= file->getSize()) {

			episodes = ++count;

			for (; count < 11; count++) {

				pixels = new unsigned char[1];
				*pixels = 0;
				screens[count + 3] = createSurface(pixels, 1, 1);

			}

		}

	}

	delete file;

	localPlayer = NULL;
	game = NULL;

	playMusic("menusng.psm");

	return;

}


Menu::~Menu () {

	int count;

	for (count = 0; count < 15; count++) SDL_FreeSurface(screens[count]);

	return;

}


int Menu::message (char *text) {

	// Display a message to the user

	usePalette(palettes[1]);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ENTER].state || controls[C_ESCAPE].state) {

			releaseControl(C_ENTER);
			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(15);

		// Draw the message
		fontmn2->showString(text, screenW >> 2, (screenH >> 1) - 16);

	}

	return E_NONE;

}


int Menu::generic (char **optionNames, int options, int *chosen) {

	// Let the user select from a menu of the given options

	int count;

	usePalette(palettes[1]);

	if (*chosen >= options) *chosen = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_UNUSED;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);

		for (count = 0; count < options; count++) {

			if (count == *chosen) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(optionNames[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - (options << 3));

			if (count == *chosen) fontmn2->restorePalette();

		}

		if (controls[C_UP].state) {

			releaseControl(C_UP);

			*chosen = (*chosen + options - 1) % options;

		}

		if (controls[C_DOWN].state) {

			releaseControl(C_DOWN);

			*chosen = (*chosen + 1) % options;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			playSound(S_ORB);

			return E_NONE;

		}

	}

	return E_NONE;

}


int Menu::textInput (char *request, char **text) {

	// Let the user to edit a text string

	char *input;
	int count, terminate, character, x;
	unsigned int cursor;

	// Create input string
	input = new char[STRING_LENGTH + 1];
	strcpy(input, *text);

	cursor = strlen(input);

	while (true) {

		character = loop(KEY_LOOP);

		if (character == E_QUIT) {

			delete[] input;

			return E_QUIT;

		}

		// Ensure there is space for another character
		if (cursor < STRING_LENGTH) {

			terminate = (input[cursor] == 0);

			// If the character is valid, add it to the input string

			if ((character == ' ') || (character == '.') ||
				((character >= '0') && (character <= '9')) ||
				((character >= 'a') && (character <= 'z'))) {

				input[cursor] = character;
				cursor++;
				if (terminate) input[cursor] = 0;

			} else if ((character >= 'A') && (character <= 'Z')) {

				input[cursor] = character | 32;
				cursor++;
				if (terminate) input[cursor] = 0;

			}

		}

		if ((character == SDLK_DELETE) && (cursor < strlen(input))) {

			for (count = cursor; count < STRING_LENGTH; count++)
				input[count] = input[count + 1];

		}

		if ((character == SDLK_BACKSPACE) && (cursor > 0)) {

			for (count = cursor - 1; count < STRING_LENGTH; count++)
				input[count] = input[count + 1];

			cursor--;

		}


		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			delete[] input;

			return E_UNUSED;

		}

		SDL_Delay(T_FRAME);

		clearScreen(15);

		// Draw the prompt
		fontmn2->showString(request, screenW >> 2, (screenH >> 1) - 16);

		// Draw the section of the text before the cursor
		fontmn2->mapPalette(240, 8, 114, 16);
		terminate = input[cursor];
		input[cursor] = 0;
		x = fontmn2->showString(input, (screenW >> 2) + 8, screenH >> 1);

		// Draw the cursor
		drawRect(x, (screenH >> 1) + 10, 8, 2, 79);

		// Draw the section of text after the cursor
		input[cursor] = terminate;
		fontmn2->showString(input + cursor, x, screenH >> 1);
		fontmn2->restorePalette();


		if (controls[C_LEFT].state) {

			releaseControl(C_LEFT);

			if (cursor > 0) cursor--;

		}

		if (controls[C_RIGHT].state) {

			releaseControl(C_RIGHT);

			if (cursor < strlen(input)) cursor++;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			playSound(S_ORB);

			// Replace the original string with the input string
			delete[] *text;
			*text = input;

			return E_NONE;

		}

	}

	delete[] input;

	return E_UNUSED;

}


int Menu::newGameDifficulty (int mode, int levelNum, int worldNum) {

	char *options[4] = {"easy", "medium", "hard", "turbo"};
	char firstLevel[11];
	SDL_Rect src, dst;
	int count;

	usePalette(palettes[1]);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);

		for (count = 0; count < 4; count++) {

			if (count == difficulty) fontmn2->mapPalette(240, 8, 114, 16);

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

		if (controls[C_UP].state) {

			releaseControl(C_UP);
			difficulty = (difficulty + 3) % 4;

		}

		if (controls[C_DOWN].state) {

			releaseControl(C_DOWN);
			difficulty = (difficulty + 1) % 4;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			playSound(S_ORB);

			sprintf(firstLevel, F_LEVEL, levelNum, worldNum);

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

			switch (game->run()) {

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


int Menu::newGameLevel (int mode) {

	int option, worldNum, levelNum;

	worldNum = levelNum = option = 0;

	usePalette(palettes[1]);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(15);

		if (option == 0) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("choose world:", 32, screenH / 3);
		fontmn2->showNumber(worldNum, 208, screenH / 3);

		if (option == 0) fontmn2->restorePalette();
		else fontmn2->mapPalette(240, 8, 114, 16);

		fontmn2->showString("choose level:", 32, (screenH << 1) / 3);
		fontmn2->showNumber(levelNum, 208, (screenH << 1) / 3);

		if (option != 0) fontmn2->restorePalette();

		if (controls[C_UP].state) {

			releaseControl(C_UP);

			option ^= 1;

		}

		if (controls[C_DOWN].state) {

			releaseControl(C_DOWN);

			option ^= 1;

		}

		if (controls[C_LEFT].state) {

			releaseControl(C_LEFT);

			if (option) levelNum = (levelNum + 9) % 10;
			else worldNum = (worldNum + 999) % 1000;

		}

		if (controls[C_RIGHT].state) {

			releaseControl(C_RIGHT);

			if (option) levelNum = (levelNum + 1) % 10;
			else worldNum = (worldNum + 1) % 1000;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			playSound(S_ORB);

			if (newGameDifficulty(mode, levelNum, worldNum) == E_QUIT)
				return E_QUIT;

			usePalette(palettes[1]);

		}

	}

	return E_NONE;

}


int Menu::newGameEpisode (int mode) {

	char *options[12] = {"episode 1", "episode 2", "episode 3", "episode 4",
		"episode 5", "episode 6", "episode a", "episode b", "episode c",
		"episode x", "bonus stage", "specific level"};
	bool exists[12];
	char check[11];
	SDL_Rect dst;
	int episode, count, worldNum;

	usePalette(palettes[2]);

	for (count = 0; count < 10; count++) {

		if (count < 6) worldNum = count * 3;
		else if ((count >= 6) && (count < 9)) worldNum = (count + 4) * 3;
		else worldNum = 50;

		sprintf(check, F_LEVEL, 0, worldNum);

		exists[count] = fileExists(check);

		if (exists[count]) restorePalette(screens[count + 3]);
		else
			SDL_SetPalette(screens[count + 3], SDL_LOGPAL, palettes[3], 0, 256);

	}

	exists[10] = false;
	exists[11] = true;

	episode = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);

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

				fontmn2->mapPalette(240, 8, 79, -80);
				drawRect((screenW >> 3) - 4, (screenH >> 1) + (count << 4) - 94,
					136, 15, 79);

			} else if (!exists[count])
				fontmn2->mapPalette(240, 8, 94, -16);

			fontmn2->showString(options[count], screenW >> 3,
				(screenH >> 1) + (count << 4) - 92);

			if ((count == episode) || (!exists[count]))
				fontmn2->mapPalette(240, 8, 9, 80);

		}

		if (controls[C_UP].state) {

			releaseControl(C_UP);

			episode = (episode + 11) % 12;

		}

		if (controls[C_DOWN].state) {

			releaseControl(C_DOWN);

			episode = (episode + 1) % 12;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			playSound(S_ORB);

			if (exists[episode]) {

				if (episode < 10) {

					if (episode < 6) worldNum = episode * 3;
					else if ((episode >= 6) && (episode < 9))
						worldNum = (episode + 4) * 3;
					else worldNum = 50;

					if (newGameDifficulty(mode, 0, worldNum) == E_QUIT)
						return E_QUIT;

				} else if (episode == 10) {

					// To do

				} else {

					if (newGameLevel(mode) == E_QUIT) return E_QUIT;

				}

				usePalette(palettes[2]);

			}

		}

	}

	return E_NONE;

}


int Menu::joinGame () {

	int ret;

	ret = textInput("ip address:", &netAddress);

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

	switch (game->run()) {

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


int Menu::loadGame () {

	// To do

	return E_NONE;

}


int Menu::setupKeyboard () {

	char *options[7] = {"up", "down", "left", "right", "jump", "fire",
		"weapon"};
	int progress, count, character;
	bool used;

	progress = 0;

	while (true) {

		character = loop(KEY_LOOP);

		if (character == E_QUIT) return E_QUIT;

		if (character > 0) {

			used = false;

			// Check if key is already in use

			for (count = 0; count < CONTROLS; count++)
				if (character == keys[count].key) {

					if (count != progress) used = true;

				}

			// If not, assign it to the current control

			if (!used) {

				keys[progress].key = character;
				progress++;

				if (progress == 7) {

					// If all controls have been assigned, return

					playSound(S_ORB);

					return E_NONE;

				}

			}

		}


		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);

		for (count = 0; count < 7; count++) {

			if (count < progress)
				fontmn2->showString("okay", (screenW >> 2) + 176,
					(screenH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press key", (screenW >> 2) + 112,
					(screenH >> 1) + (count << 4) - 56);

				fontmn2->restorePalette();

			}

		}

	}

	return E_NONE;

}


int Menu::setupJoystick () {

	char *options[7] = {"up", "down", "left", "right", "jump", "fire",
		"weapon"};
	int progress, count, control;
	bool used;

	progress = 0;

	while (true) {

		control = loop(JOYSTICK_LOOP);

		if (control == E_QUIT) return E_QUIT;

		switch (control & 0xF00) {

			case JOYSTICKB:

				used = false;

				// Check if the button is already in use

				for (count = 0; count < CONTROLS; count++)
					if ((control & 0xFF) == buttons[count].button) {

						if (count != progress) used = true;

					}

				// If not, assign it to the current control

				if (!used) {

					buttons[progress].button = control & 0xFF;
					progress++;

					if (progress == 7) {

						// If all controls have been assigned, return

						playSound(S_ORB);

						return E_NONE;

					}

				}

				break;

			case JOYSTICKANEG:

				used = false;

				// Check if the arrow is already in use

				for (count = 0; count < CONTROLS; count++)
					if (((control & 0xFF) == axes[count].axis) &&
						!axes[count].direction) {

						if (count != progress) used = true;

					}

				// If not, assign it to the current control

				if (!used) {

					axes[progress].axis = control & 0xFF;
					axes[progress].direction = 0;
					progress++;

					if (progress == 7) {

						// If all controls have been assigned, return

						playSound(S_ORB);

						return E_NONE;

					}

				}

				break;

			case JOYSTICKAPOS:

				used = false;

				// Check if the arrow is already in use

				for (count = 0; count < CONTROLS; count++)
					if (((control & 0xFF) == axes[count].axis) &&
						axes[count].direction) {

						if (count != progress) used = true;

					}

				// If not, assign it to the current control

				if (!used) {

					axes[progress].axis = control & 0xFF;
					axes[progress].direction = 1;
					progress++;

					if (progress == 7) {

						// If all controls have been assigned, return

						playSound(S_ORB);

						return E_NONE;

					}

				}

				break;

		}

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);

		for (count = 0; count < 7; count++) {

			if (count < progress)
				fontmn2->showString("okay", (screenW >> 2) + 176,
					(screenH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], screenW >> 2,
				(screenH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press control", (screenW >> 2) + 112,
					(screenH >> 1) + (count << 4) - 56);

				fontmn2->restorePalette();

			}

		}

	}

	return E_NONE;

}


int Menu::setupResolution () {

#ifndef FULLSCREEN_ONLY

	int widthOptions[] = {320, 400, 512, 640, 720, 768, 800, 960, 1024, 1152,
		1280, 1440, 1600, 1920};
	int heightOptions[] = {200, 240, 300, 384, 400, 480, 576, 600, 720, 768,
		800, 864, 900, 960, 1024, 1080, 1200};
	SDL_Rect **resolutions;
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

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			return E_NONE;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);


		// Show screen corners
		drawRect(0, 0, 32, 32, 79);
		drawRect(screenW - 32, 0, 32, 32, 79);
		drawRect(screenW - 32, screenH - 32, 32, 32, 79);
		drawRect(0, screenH - 32, 32, 32, 79);


		fontmn2->showString("x", (screenW >> 2) + 40, screenH >> 1);

		if (dimension == 0) fontmn2->mapPalette(240, 8, 114, 16);

		fontmn2->showNumber(screenW, (screenW >> 2) + 32, screenH >> 1);

		if (dimension == 0) fontmn2->restorePalette();
		else fontmn2->mapPalette(240, 8, 114, 16);

		fontmn2->showNumber(screenH, (screenW >> 2) + 104, screenH >> 1);

		if (dimension != 0) fontmn2->restorePalette();


		count = 0;

		if (controls[C_LEFT].state) {

			releaseControl(C_LEFT);

			dimension = !dimension;

		}

		if (controls[C_RIGHT].state) {

			releaseControl(C_RIGHT);

			dimension = !dimension;

		}

		if (controls[C_UP].state) {

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

		if (controls[C_DOWN].state) {

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

			playSound(S_ORB);

			screen = SDL_SetVideoMode(screenW, screenH, 8,
				(fullscreen? SDL_FULLSCREEN: SDL_RESIZABLE) | SDL_DOUBLEBUF |
				SDL_HWSURFACE | SDL_HWPALETTE);

			SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
			SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

		}

	}

#endif

	return E_NONE;

}


int Menu::setup () {

	char *setupOptions[4] = {"character", "keyboard", "joystick", "resolution"};
	char *setupCharacterOptions[5] = {"name", "fur", "bandana", "gun",
		"wristband"};
	char *setupCharacterColOptions[8] = {"white", "red", "orange", "yellow",
		"green", "blue", "animation 1", "animation 2"};
	unsigned char setupCharacterCols[8] = {PC_WHITE, PC_RED, PC_ORANGE,
		PC_YELLOW, PC_LGREEN, PC_BLUE, PC_SANIM, PC_LANIM};
	int ret;
	int option, suboption, subsuboption;

	option = 0;

	while (true) {

		ret = generic(setupOptions, 4, &option);

		if (ret == E_UNUSED) return E_NONE;
		if (ret < 0) return ret;

		switch (option) {

			case 0:

				suboption = 0;

				while (true) {

					ret = generic(setupCharacterOptions, 5, &suboption);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					switch (suboption) {

						case 0: // Character name

							textInput("character name:", &characterName);

							break;

						default: // Character colour

							subsuboption = 0;
							ret = generic(setupCharacterColOptions, 8,
								&subsuboption);

							if (ret == E_QUIT) return E_QUIT;

							if (ret == E_NONE)
								characterCols[suboption - 1] =
									setupCharacterCols[subsuboption];

							break;

					}

				}

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

	return E_NONE;

}


int Menu::run () {

	char *newGameOptions[5] = {"new single player game", "new co-op game",
		"new battle", "new team battle", "join game"};
	Scene *scene;
	SDL_Rect src, dst;
	int option, suboption;
	unsigned int idleTime;
	int ret;

	option = suboption = 0;

	usePalette(palettes[0]);

	// Demo timeout
	idleTime = SDL_GetTicks() + T_DEMO;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			option = 5;

		}
		if (controls[C_UP].state) {

			releaseControl(C_UP);

			option = (option + 5) % 6;

		}

		if (controls[C_DOWN].state) {

			releaseControl(C_DOWN);

			option = (option + 1) % 6;

		}

		if (controls[C_ENTER].state) {

			releaseControl(C_ENTER);

			playSound(S_ORB);

			switch(option) {

				case 0: // New game

					while (true) {

						ret = generic(newGameOptions, 5, &suboption);

						if (ret == E_QUIT) return E_QUIT;
						if (ret < 0) break;

						switch (suboption) {

							case 0:

								if (newGameEpisode(M_SINGLE) == E_QUIT)
									return E_QUIT;

								break;

							case 1:

								if (newGameEpisode(M_COOP) == E_QUIT)
									return E_QUIT;

								break;

							case 2:

								if (newGameEpisode(M_BATTLE) == E_QUIT)
									return E_QUIT;

								break;

							case 3:

								if (newGameEpisode(M_TEAMBATTLE) == E_QUIT)
									return E_QUIT;

								break;

							case 4:

								if (joinGame() == E_QUIT) return E_QUIT;

								break;

						}

					}

					break;

				case 1: // Load game

					if (loadGame() == E_QUIT) return E_QUIT;

					break;

				case 2: // Instructions

					scene = new Scene("instruct.0sc");

					if (scene->run() == E_QUIT) {

						delete scene;

						return E_QUIT;

					}

					delete scene;

					break;

				case 3: // Setup options

					if (setup() == E_QUIT) return E_QUIT;

					break;

				case 4: // Order info

					scene = new Scene("order.0sc");

					if (scene->run() == E_QUIT) {

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
			idleTime = SDL_GetTicks() + T_DEMO;

		}


		if (idleTime <= SDL_GetTicks()) {

			// Use the menu palette for the level loading screen
			usePalette(palettes[1]);

			// Create the player
			nPlayers = 1;
			localPlayer = players = new Player[1];
			localPlayer->init(characterName, NULL, 0);


			// Load the macro

			try {

				level = new DemoLevel("macro.2");

			} catch (int e) {

				delete[] players;
				localPlayer = NULL;

				break;

			}

			// Play the level
			if (level->run() == E_QUIT) {

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

			idleTime = SDL_GetTicks() + T_DEMO;

		}

		SDL_Delay(T_FRAME);

		clearScreen(28);

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

	}

	return E_NONE;

}


