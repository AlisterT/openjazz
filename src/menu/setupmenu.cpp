
/**
 *
 * @file setupmenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * 26th July 2009: Renamed menusetup.cpp to setupmenu.cpp
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
 * @section Description
 * Deals with the running of setup menus.
 *
 */


#include "menu.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/player.h"
#include "loop.h"


int SetupMenu::setupKeyboard () {

	const char *options[PCONTROLS] = {"up", "down", "left", "right", "jump", "swim up", "fire", "weapon"};
	int progress, count, character;

	progress = 0;

	while (true) {

		character = loop(SET_KEY_LOOP);

		if (character == E_QUIT) return E_QUIT;

		if (character == controls.getKey(C_ESCAPE)) return E_NONE;

		if (character > 0) {

			// If this is a navigation controls (up, down, or enter),
			// make sure it's not the same as other navigation controls

			if (((progress != C_UP) &&
				(progress != C_DOWN) &&
				(progress != C_ENTER)) ||
				(controls.getKey(progress) == character) ||
				((controls.getKey(C_UP) != character) &&
				(controls.getKey(C_DOWN) != character) &&
				(controls.getKey(C_ENTER) != character))) {

				controls.setKey(progress, character);
				progress++;

				if (progress == PCONTROLS) {

					// If all controls have been assigned, return

					playSound(S_ORB);

					return E_NONE;

				}

			}

		}


		SDL_Delay(T_FRAME);

		video.clearScreen(0);

		for (count = 0; count < PCONTROLS; count++) {

			if (count < progress)
				fontmn2->showString("okay", (canvasW >> 2) + 176,
					(canvasH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], canvasW >> 2,
				(canvasH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press key", (canvasW >> 2) + 112,
					(canvasH >> 1) + (count << 4) - 56);

				fontmn2->restorePalette();

			}

		}

	}

	return E_NONE;

}


int SetupMenu::setupJoystick () {

	const char *options[PCONTROLS] = {"up", "down", "left", "right", "jump", "swim up", "fire", "weapon"};
	int progress, count, control;

	progress = 0;

	while (true) {

		control = loop(SET_JOYSTICK_LOOP);

		if (control == E_QUIT) return E_QUIT;

		switch (control & 0xF00) {

			case JOYSTICKB:

				// If this is a navigation controls (up, down, or enter),
				// make sure it's not the same as other navigation controls

				if (((progress != C_UP) &&
					(progress != C_DOWN) &&
					(progress != C_ENTER)) ||
					(controls.getButton(progress) == (control & 0xFF)) ||
					((controls.getButton(C_UP) != (control & 0xFF)) &&
					(controls.getButton(C_DOWN) != (control & 0xFF)) &&
					(controls.getButton(C_ENTER) != (control & 0xFF)))) {

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

				// If this is a navigation controls (up, down, or enter),
				// make sure it's not the same as other navigation controls

				if (((progress != C_UP) &&
					(progress != C_DOWN) &&
					(progress != C_ENTER)) ||
					((controls.getAxis(progress) == (control & 0xFF)) && !controls.getAxisDirection(progress)) ||
					(((controls.getAxis(C_UP) != (control & 0xFF)) || controls.getAxisDirection(C_UP)) &&
					((controls.getAxis(C_DOWN) != (control & 0xFF)) || controls.getAxisDirection(C_DOWN)) &&
					((controls.getAxis(C_ENTER) != (control & 0xFF)) || controls.getAxisDirection(C_ENTER)))) {

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

				// If this is a navigation controls (up, down, or enter),
				// make sure it's not the same as other navigation controls

				if (((progress != C_UP) &&
					(progress != C_DOWN) &&
					(progress != C_ENTER)) ||
					((controls.getAxis(progress) == (control & 0xFF)) && controls.getAxisDirection(progress)) ||
					(((controls.getAxis(C_UP) != (control & 0xFF)) || !controls.getAxisDirection(C_UP)) &&
					((controls.getAxis(C_DOWN) != (control & 0xFF)) || !controls.getAxisDirection(C_DOWN)) &&
					((controls.getAxis(C_ENTER) != (control & 0xFF)) || !controls.getAxisDirection(C_ENTER)))) {

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

		video.clearScreen(0);

		for (count = 0; count < 7; count++) {

			if (count < progress)
				fontmn2->showString("okay", (canvasW >> 2) + 176, (canvasH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], canvasW >> 2, (canvasH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press control", (canvasW >> 2) + 112, (canvasH >> 1) + (count << 4) - 56);

				fontmn2->restorePalette();

			}

		}

	}

	return E_NONE;

}


int SetupMenu::setupResolution () {

	int widthOptions[] = {320, 400, 512, 640, 720, 768, 800, 960, 1024, 1152,
		1280, 1440, 1600, 1920};
	int heightOptions[] = {200, 240, 300, 384, 400, 480, 576, 600, 720, 768,
		800, 864, 900, 960, 1024, 1080, 1200};
	SDL_Rect **resolutions;
	int count, screenW, screenH, maxW, maxH;
	bool dimension;

	screenW = video.getWidth();
	screenH = video.getHeight();

	dimension = false;

#ifndef FULLSCREEN_ONLY
	if (!video.isFullscreen())
		resolutions = SDL_ListModes(NULL, WINDOWED_FLAGS);
	else
#endif
		resolutions = SDL_ListModes(NULL, FULLSCREEN_FLAGS);


#if defined(CAANOO) ||defined(WIZ) || defined(GP2X) || defined(DINGOO)
	maxW = 320;
	maxH = 240;
#else
	if (resolutions == (SDL_Rect **)(-1)) {

		maxW = 1920;
		maxH = 1200;

	} else {

		maxW = SW;
		maxH = SH;

		for (count = 0; resolutions[count] != NULL; count++) {

			if (resolutions[count]->w > maxW) maxW = resolutions[count]->w;
			if (resolutions[count]->h > maxH) maxH = resolutions[count]->h;

		}
	}
#endif

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_ENTER)) return E_NONE;

		SDL_Delay(T_FRAME);

		video.clearScreen(0);


		// Show screen corners
		drawRect(0, 0, 32, 32, 79);
		drawRect(canvasW - 32, 0, 32, 32, 79);
		drawRect(canvasW - 32, canvasH - 32, 32, 32, 79);
		drawRect(0, canvasH - 32, 32, 32, 79);


		// X
		fontmn2->showString("x", (canvasW >> 2) + 40, canvasH >> 1);

		if (!dimension) fontmn2->mapPalette(240, 8, 114, 16);

		// Width
		fontmn2->showNumber(screenW, (canvasW >> 2) + 32, canvasH >> 1);

		if (!dimension) fontmn2->restorePalette();
		else fontmn2->mapPalette(240, 8, 114, 16);

		// Height
		fontmn2->showNumber(screenH, (canvasW >> 2) + 104, canvasH >> 1);

		if (dimension) fontmn2->restorePalette();


		count = 0;

		if (controls.release(C_LEFT)) dimension = !dimension;

		if (controls.release(C_RIGHT)) dimension = !dimension;

		if (controls.release(C_UP)) {

			if ((!dimension) && (screenW < maxW)) {

				while (screenW >= widthOptions[count]) count++;

				screenW = widthOptions[count];

			}

			if (dimension && (screenH < maxH)) {

				while (screenH >= heightOptions[count]) count++;

				screenH = heightOptions[count];

			}

		}

		if (controls.release(C_DOWN)) {

			if ((!dimension) && (screenW > SW)) {

				count = 13;

				while (screenW <= widthOptions[count]) count--;

				screenW = widthOptions[count];
				count = -1;

			}

			if (dimension && (screenH > SH)) {

				count = 16;

				while (screenH <= heightOptions[count]) count--;

				screenH = heightOptions[count];
				count = -1;

			}

		}

		// Check for a resolution change
		if (count) {

			playSound(S_ORB);
			video.create(screenW, screenH);

		}

	}

	return E_NONE;

}


#ifdef SCALE
int SetupMenu::setupScaling () {

	int scaleFactor;

	scaleFactor = video.getScaleFactor();

	if ( scaleFactor < MIN_SCALE || scaleFactor > MAX_SCALE )
		scaleFactor = 1;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_ENTER)) return E_NONE;

		SDL_Delay(T_FRAME);

		video.clearScreen(0);


		// Show screen corners
		drawRect(0, 0, 32, 32, 79);
		drawRect(canvasW - 32, 0, 32, 32, 79);
		drawRect(canvasW - 32, canvasH - 32, 32, 32, 79);
		drawRect(0, canvasH - 32, 32, 32, 79);



		fontmn2->mapPalette(240, 8, 114, 16);

		// Scale
		fontmn2->showNumber(video.getScaleFactor(), (canvasW >> 2) + 32, canvasH >> 1);

		// X
		fontmn2->showString("x", (canvasW >> 2) + 40, canvasH >> 1);

		fontmn2->restorePalette();


		if ((controls.release(C_DOWN) || controls.release(C_LEFT)) && (scaleFactor > MIN_SCALE)) scaleFactor--;

		if ((controls.release(C_UP) || controls.release(C_RIGHT)) && (scaleFactor < MAX_SCALE)) scaleFactor++;

		// Check for a scaling change
		if (scaleFactor != video.getScaleFactor()) {

			playSound(S_ORB);
			video.setScaleFactor(scaleFactor);

		}

	}

	return E_NONE;

}
#endif


int SetupMenu::setupSound () {

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_ENTER)) return E_NONE;

		SDL_Delay(T_FRAME);

		video.clearScreen(0);


		// Volume
		fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("effect volume", canvasW >> 2, canvasH >> 1);
		fontmn2->restorePalette();

		drawRect((canvasW >> 2) + 120, canvasH >> 1, soundsVolume >> 1, 11, 175);

		if (controls.release(C_LEFT)) soundsVolume -= 4;
		if (soundsVolume < 0) soundsVolume = 0;

		if (controls.release(C_RIGHT)) soundsVolume += 4;
		if (soundsVolume > MAX_VOLUME) soundsVolume = MAX_VOLUME;

	}

	return E_NONE;

}


int SetupMenu::setup () {

	const char *setupOptions[6] = {"character", "keyboard", "joystick", "resolution", "scaling", "sound"};
	const char *setupCharacterOptions[5] = {"name", "fur", "bandana", "gun", "wristband"};
	const char *setupCharacterColOptions[8] = {"white", "red", "orange", "yellow", "green", "blue", "animation 1", "animation 2"};
	const unsigned char setupCharacterCols[8] = {PC_GREY, PC_RED, PC_ORANGE, PC_YELLOW, PC_LGREEN, PC_BLUE, PC_SANIM, PC_LANIM};
	int ret;
	int option, suboption, subsuboption;

	option = 0;

	while (true) {

		ret = generic(setupOptions, 6, option);

		if (ret == E_UNUSED) return E_NONE;
		if (ret < 0) return ret;

		switch (option) {

			case 0:

				suboption = 0;

				while (true) {

					ret = generic(setupCharacterOptions, 5, suboption);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					switch (suboption) {

						case 0: // Character name

							textInput("character name:", characterName);

							break;

						default: // Character colour

							subsuboption = 0;
							ret = generic(setupCharacterColOptions, 8, subsuboption);

							if (ret == E_QUIT) return E_QUIT;

							if (ret == E_NONE)
								characterCols[suboption - 1] = setupCharacterCols[subsuboption];

							break;

					}

				}

				break;

			case 1:

#if !defined(CAANOO) && !defined(WIZ) && !defined(GP2X)
				setupKeyboard();
#else
				message("FEATURE NOT AVAILABLE");
#endif

				break;

			case 2:

#if !defined(DINGOO)
				setupJoystick();

#else
				message("FEATURE NOT AVAILABLE");
#endif

				break;

			case 3:

				setupResolution();

				break;

			case 4:

#ifdef SCALE
				setupScaling();
#else
				message("FEATURE NOT AVAILABLE");
#endif

				break;

			case 5:

				setupSound();

				break;

		}

	}

	return E_NONE;

}



