
/*
 *
 * setupmenu.cpp
 *
 * 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menusetup.cpp to setupmenu.cpp
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
 * Deals with the running of setup menus.
 *
 */


#include "menu.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/player.h"


int Menu::setupKeyboard () {

	const char *options[7] = {"up", "down", "left", "right", "jump", "fire",
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
				if (character == controls.getKey(count)) {

					if (count != progress) used = true;

				}

			// If not, assign it to the current control

			if (!used) {

				controls.setKey(progress, character);
				progress++;

				if (progress == 7) {

					// If all controls have been assigned, return

					playSound(S_ORB);

					return E_NONE;

				}

			}

		}


		if (controls.release(C_ESCAPE)) return E_NONE;

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

	const char *options[7] = {"up", "down", "left", "right", "jump", "fire",
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
					if ((control & 0xFF) == controls.getButton(count)) {

						if (count != progress) used = true;

					}

				// If not, assign it to the current control

				if (!used) {

					controls.setButton(progress, control & 0xFF);
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
					if (((control & 0xFF) == controls.getAxis(count)) &&
						!controls.getAxisDirection(count)) {

						if (count != progress) used = true;

					}

				// If not, assign it to the current control

				if (!used) {

					controls.setAxis(progress, control & 0xFF, false);
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
					if (((control & 0xFF) == controls.getAxis(count)) &&
						controls.getAxisDirection(count)) {

						if (count != progress) used = true;

					}

				// If not, assign it to the current control

				if (!used) {

					controls.setAxis(progress, control & 0xFF, true);
					progress++;

					if (progress == 7) {

						// If all controls have been assigned, return

						playSound(S_ORB);

						return E_NONE;

					}

				}

				break;

		}

		if (controls.release(C_ESCAPE)) return E_NONE;

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

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_ENTER)) return E_NONE;

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

		if (controls.release(C_LEFT)) dimension = !dimension;

		if (controls.release(C_RIGHT)) dimension = !dimension;

		if (controls.release(C_UP)) {

			if ((dimension == 0) && (screenW < maxW)) {

				while (screenW >= widthOptions[count]) count++;

				screenW = widthOptions[count];

			}

			if ((dimension == 1) && (screenH < maxH)) {

				while (screenH >= heightOptions[count]) count++;

				screenH = heightOptions[count];

			}

		}

		if (controls.release(C_DOWN)) {

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

			if (fullscreen) createFullscreen();
			else createWindow();

		}

	}

#endif

	return E_NONE;

}


int Menu::setup () {

	const char *setupOptions[4] = {"character", "keyboard", "joystick",
		"resolution"};
	const char *setupCharacterOptions[5] = {"name", "fur", "bandana", "gun",
		"wristband"};
	const char *setupCharacterColOptions[8] = {"white", "red", "orange",
		"yellow", "green", "blue", "animation 1", "animation 2"};
	const unsigned char setupCharacterCols[8] = {PC_WHITE, PC_RED, PC_ORANGE,
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



