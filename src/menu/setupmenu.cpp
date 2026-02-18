
/**
 *
 * @file setupmenu.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd of August 2005: Created menu.c
 * - 3rd of February 2009: Renamed menu.c to menu.cpp
 * - 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * - 26th July 2009: Renamed menusetup.cpp to setupmenu.cpp
 * - 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
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
#include "setup.h"
#include "util.h"
#include "platforms/platforms.h"

#ifndef NO_KEYBOARD_CFG
/**
 * Run the keyboard setup menu.
 *
 * @return Error code
 */
int SetupMenu::setupKeyboard () {

	const char *options[PCONTROLS] = {"up", "down", "left", "right", "jump", "swim up", "fire", "weapon"};
	int progress, x, y, count;

	progress = 0;

	while (true) {

		int character = loop(SET_KEY_LOOP);

		if (character == E_QUIT) return E_QUIT;

		if (character == controls.getKey(C_ESCAPE)) return E_NONE;

		if (controls.getCursor(x, y) &&
			(x < 100) && (y >= canvasH - 12) &&
			controls.wasCursorReleased()) return E_NONE;

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
					playConfirmSound();
					return E_NONE;

				}

			}

		}


		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);

		fontmn2->showString("KEYBOARD CONFIGURATION", (canvasW >> 2), (canvasH >> 1) - 80);

		for (count = 0; count < PCONTROLS; count++) {

			if (count < progress)
				fontmn2->showString(controls.getKeyName(count), (canvasW >> 2) + 176,
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

		showEscString();

	}

	return E_NONE;

}
#endif

#ifndef NO_CONTROLLER_CFG
/**
 * Run the joystick setup menu.
 *
 * @return Error code
 */
int SetupMenu::setupJoystick () {

	const char *options[PCONTROLS] = {"up", "down", "left", "right", "jump", "swim up", "fire", "weapon"};
	int x, y;

	int progress = 0;

	while (true) {

		int control = loop(SET_JOYSTICK_LOOP);

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

					if (progress == PCONTROLS) {

						// If all controls have been assigned, return
						playConfirmSound();
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

					if (progress == PCONTROLS) {

						// If all controls have been assigned, return
						playConfirmSound();
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

					if (progress == PCONTROLS) {

						// If all controls have been assigned, return
						playConfirmSound();
						return E_NONE;

					}

				}

				break;

			case JOYSTICKHUP:
			case JOYSTICKHLFT:
			case JOYSTICKHRHT:
			case JOYSTICKHDWN:

				int direction = 0;
				switch(control & 0xF00) {
					case JOYSTICKHUP:  direction = SDL_HAT_UP;    break;
					case JOYSTICKHLFT: direction = SDL_HAT_LEFT;  break;
					case JOYSTICKHRHT: direction = SDL_HAT_RIGHT; break;
					case JOYSTICKHDWN: direction = SDL_HAT_DOWN;  break;
				}

				// If this is a navigation controls (up, down, or enter),
				// make sure it's not the same as other navigation controls

				if (((progress != C_UP) &&
					(progress != C_DOWN) &&
					(progress != C_ENTER)) ||
					((controls.getHat(progress) == (control & 0xFF)) && (controls.getHatDirection(progress) == direction)) ||
					(((controls.getHat(C_UP) != (control & 0xFF)) || (controls.getHatDirection(C_UP) != direction)) &&
					((controls.getHat(C_DOWN) != (control & 0xFF)) || (controls.getHatDirection(C_DOWN) != direction)) &&
					((controls.getHat(C_ENTER) != (control & 0xFF)) || (controls.getHatDirection(C_ENTER) != direction)))) {

					controls.setHat(progress, control & 0xFF, direction);
					progress++;

					if (progress == PCONTROLS) {

						// If all controls have been assigned, return
						playConfirmSound();
						return E_NONE;

					}

				}

				break;

		}

		if (controls.release(C_ESCAPE)) return E_NONE;

		if ((controls.getCursor(x, y) &&
			(x < 100) && (y >= canvasH - 12) &&
			controls.wasCursorReleased())) return E_NONE;


		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);

		fontmn2->showString("JOYSTICK CONFIGURATION", (canvasW >> 2), (canvasH >> 1) - 80);

		for (int count = 0; count < PCONTROLS; count++) {

			if (count < progress)
				fontmn2->showString("okay", (canvasW >> 2) + 176, (canvasH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], canvasW >> 2, (canvasH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press control", (canvasW >> 2) + 112, (canvasH >> 1) + (count << 4) - 56);

				fontmn2->restorePalette();

			}

		}

		showEscString();

	}

	return E_NONE;

}
#endif

#if !defined(NO_RESIZE) || defined(SCALE)
/**
 * Run the resolution/scaling setup menu.
 *
 * @return Error code
 */
int SetupMenu::setupVideo () {
	int widthOptions[] = {SW, 352, 384, 400, 480, 512, 640, 720, 768, 800, 960,
	    1024, 1152, 1280, 1366, 1400, 1440, 1600, 1680, 1920, 2048, 2560, 3200,
	    3440, 3840, 4096, MAX_SCREEN_WIDTH};
	int heightOptions[] = {SH, 240, 288, 300, 320, 384, 400, 480, 576, 600, 720,
	    768, 800, 864, 900, 960, 1024, 1050, 1080, 1152, 1200, 1440, 1536, 1600,
	    2048, 2160, MAX_SCREEN_HEIGHT};
	const char *methodString[4] = { "nearest", "bilinear", "scalex", "hqx"};
	int scaleFactor = video.getScaleFactor();
	scalerType scaleMethod = video.getScaleMethod();
	int screenW, screenH, oldscreenW, oldscreenH, x, y;
	screenW = oldscreenW = video.getWidth();
	screenH = oldscreenH = video.getHeight();
	int selection = 0;
	bool resOK = true;

	char scaleString[3] = "Yx";
	bool fillScaleString = true;
	char resString[12] = "XXXX x YYYY";
	bool fillResString = true;

	// helpers
	auto changeWidth = [&] (bool isPositive) {
		int count = 0;
		if(isPositive && (screenW < video.getMaxWidth())) {
			while (screenW >= widthOptions[count])
				count++;

			screenW = widthOptions[count];
		} else if (!isPositive && (screenW > video.getMinWidth())) {
			count = sizeof(widthOptions)/sizeof(widthOptions[0]) - 1;
			while (screenW <= widthOptions[count])
				count--;

			screenW = widthOptions[count];
		}
	};
	auto changeHeight = [&] (bool isPositive) {
		int count = 0;
		if(isPositive && (screenH < video.getMaxHeight())) {
			while (screenH >= heightOptions[count])
				count++;

			screenH = heightOptions[count];
		} else if (!isPositive && (screenH > video.getMinHeight())) {
			count = sizeof(heightOptions)/sizeof(heightOptions[0]) - 1;
			while (screenH <= heightOptions[count])
				count--;

			screenH = heightOptions[count];
		}
	};
	auto changeScaleFactor = [&] (bool isPositive) {
		if(isPositive && scaleFactor < MAX_SCALE)
			scaleFactor++;
		else if (!isPositive && scaleFactor > MIN_SCALE)
			scaleFactor--;
	};
	auto changeScaleMethod = [&] (bool isPositive) {
		// TODO
#if 0
		if (isPositive && scaleMethod != scalerType::hqx)
			scaleMethod = static_cast<scalerType>(+scaleMethod + 1);
		else if(!isPositive && scaleMethod != scalerType::None)
			scaleMethod = static_cast<scalerType>(+scaleMethod - 1);
#else
	#if OJ_SDL3 || OJ_SDL2
	if(scaleMethod != scalerType::None)
		scaleMethod = scalerType::None;
	else
		scaleMethod = scalerType::Bilinear;
	#else
	if(scaleMethod != scalerType::None)
		scaleMethod = scalerType::None;
	else
		scaleMethod = scalerType::Scale2x;
	#endif
#endif
	};

	auto switchPalette = [] (bool activate) {
		if(activate)
			fontmn2->mapPalette(240, 8, 114, 16);
		else
			fontmn2->restorePalette();
	};

	// sanitize
	scaleFactor = CLAMP(scaleFactor, MIN_SCALE, MAX_SCALE);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.getCursor(x, y)) {

			if ((x >= 32) && (x < 132) && (y >= canvasH - 12)
				&& controls.wasCursorReleased())
				return E_NONE;

			// TODO: selection
		}

		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);


		// Show screen corners
		video.drawRect(0, 0, 32, 32, 79);
		video.drawRect(canvasW - 32, 0, 32, 32, 79);
		video.drawRect(canvasW - 32, canvasH - 32, 32, 32, 79);
		video.drawRect(0, canvasH - 32, 32, 32, 79);

		fontmn2->showString("VIDEO OPTIONS", (canvasW >> 2), (canvasH >> 1) - 80);

		// Game Resolution
		if(fillResString) {
			snprintf(resString, sizeof(resString), "%d x %d", canvasW, canvasH);
			fillResString = false;
		}
		video.drawRect((canvasW >> 2) - 3, (canvasH >> 1) - 50, 216, 14, 46);
		fontmn2->showString("game res:", (canvasW >> 2), (canvasH >> 1) - 48);
		fontmn2->showString(resString, (canvasW >> 2) + 88, (canvasH >> 1) - 48);


		switchPalette(selection == 0);

		// Width
		fontmn2->showString("width:", (canvasW >> 2), (canvasH >> 1) - 16);
		fontmn2->showNumber(screenW, (canvasW >> 2) + 128, (canvasH >> 1) - 16);

		switchPalette(selection == 1);

		// Height
		fontmn2->showString("height:", (canvasW >> 2), canvasH >> 1);
		fontmn2->showNumber(screenH, (canvasW >> 2) + 128, canvasH >> 1);

		switchPalette(selection == 2);

		// Factor
		if(fillScaleString) {
			snprintf(scaleString, sizeof(scaleString), "%dx", scaleFactor);
			fillScaleString = false;
		}
		fontmn2->showString("scale:", (canvasW >> 2), (canvasH >> 1) + 16);
		fontmn2->showString(scaleString, (canvasW >> 2) + 88, (canvasH >> 1) + 16);

		switchPalette(selection == 3);

		// Method
		fontmn2->showString("method:", (canvasW >> 2), (canvasH >> 1) + 32);
		fontmn2->showString(methodString[+scaleMethod], (canvasW >> 2) + 88, (canvasH >> 1) + 32);

		switchPalette(false);

		if (controls.release(C_UP)) {
			if(selection > 0)
				selection--;
			else
				selection = 3;
		}

		if (controls.release(C_DOWN)) {
			if(selection < 3)
				selection++;
			else
				selection = 0;
		}

		bool hasPressedRight = controls.release(C_RIGHT);
		if (controls.release(C_LEFT) || hasPressedRight) {
			switch(selection) {
			case 0:
				changeWidth(hasPressedRight);
				break;
			case 1:
				changeHeight(hasPressedRight);
				break;
			case 2:
				changeScaleFactor(hasPressedRight);
				fillScaleString = true;
				break;
			case 3:
				changeScaleMethod(hasPressedRight);
				break;
			}
			resOK = true;
		}

		// Check for a resolution or scaling change
		if (screenH != oldscreenH || screenW != oldscreenW ||
			scaleFactor != video.getScaleFactor() || scaleMethod != video.getScaleMethod()) {

			fontmn2->showString(resOK ? "press enter to apply" : "invalid resolution!",
				(canvasW >> 2), (canvasH >> 1) + 56);

			// Apply resolution change
			if (controls.release(C_ENTER)) {
				playConfirmSound();

				if (video.reset(screenW, screenH)) {
					// New resolution is ok
					oldscreenW = screenW;
					oldscreenH = screenH;

					video.setScaling(scaleFactor, scaleMethod);
					scaleFactor = video.getScaleFactor();
					scaleMethod = video.getScaleMethod();

					fillResString = true;
					fillScaleString = true;
				} else {
					// It failed, reset to sanity
					video.reset(oldscreenW, oldscreenH);
					resOK = false;
				}
			}
		}

		fontbig->showString(ESCAPE_STRING, 35, canvasH - 12);
	}

	return E_NONE;
}
#endif


/**
 * Run the audio setup menu.
 *
 * @return Error code
 */
int SetupMenu::setupAudio () {

	int x, y;
	bool soundActive;

	soundActive = false;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_ENTER)) return E_NONE;

		if (controls.getCursor(x, y)) {

			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;

			x -= (canvasW >> 2) + 128;
			y -= canvasH >> 1;

			if ((x >= 0) && (x < (MAX_VOLUME >> 1)) && (y >= 0) && (y < 11)) setMusicVolume(x << 1);
			if ((x >= 0) && (x < (MAX_VOLUME >> 1)) && (y >= 16) && (y < 27)) setSoundVolume(x << 1);

			if (controls.wasCursorReleased()) playConfirmSound();

		}

		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);

		fontmn2->showString("AUDIO OPTIONS", (canvasW >> 2), (canvasH >> 1) - 80);

		// Music Volume
		if (!soundActive) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("music volume", canvasW >> 2, canvasH >> 1);
		fontmn2->restorePalette();

		video.drawRect((canvasW >> 2) + 128, canvasH >> 1, getMusicVolume() >> 1, 11, 175);

		// Sound Volume
		if (soundActive) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("effect volume", canvasW >> 2, (canvasH >> 1) + 16);
		fontmn2->restorePalette();

		video.drawRect((canvasW >> 2) + 128, (canvasH >> 1) + 16, getSoundVolume() >> 1, 11, 175);

		if (controls.release(C_UP)) soundActive = !soundActive;

		if (controls.release(C_DOWN)) soundActive = !soundActive;

		if (controls.release(C_LEFT)) {

			if (soundActive) setSoundVolume(getSoundVolume() - 4);
			else setMusicVolume(getMusicVolume() - 4);

			playConfirmSound();

		}

		if (controls.release(C_RIGHT)) {

			if (soundActive) setSoundVolume(getSoundVolume() + 4);
			else setMusicVolume(getMusicVolume() + 4);

			playConfirmSound();

		}

		showEscString();

	}

	return E_NONE;

}


/**
 * Run the setup menu.
 *
 * @return Error code
 */
int SetupMenu::setupMain () {

	const char* setupOptions[6] = {"character", "keyboard", "joystick", "video", "audio", "gameplay"};
	const char* setupCharacterOptions[5] = {"name", "fur", "bandana", "gun", "wristband"};
	const char* setupCharacterColOptions[8] = {"white", "red", "orange", "yellow", "green", "blue", "animation 1", "animation 2"};
	const unsigned char setupCharacterCols[8] = {PC_GREY, PC_RED, PC_ORANGE, PC_YELLOW, PC_LGREEN, PC_BLUE, PC_SANIM, PC_LANIM};
	const char* setupModsOff[3] = {"slow motion: off", "extra items: take", "bird limit: one" };
	const char* setupModsOn[3] = {"slow motion: on", "extra items: leave", "bird limit: no" };
	const char* setupMods[3];
	int ret;
	int option, suboption, subsuboption;

	option = 0;

	setupMods[0] = (setup.slowMotion? setupModsOn[0]: setupModsOff[0]);
	setupMods[1] = (setup.leaveUnneeded? setupModsOn[1]: setupModsOff[1]);
	setupMods[2] = (setup.manyBirds? setupModsOn[2]: setupModsOff[2]);

	video.setPalette(menuPalette);

	while (true) {

		ret = generic("SETUP OPTIONS", setupOptions, 6, option);

		if (ret == E_RETURN) return E_NONE;
		if (ret < 0) return ret;

		switch (option) {

			case 0:

				suboption = 0;

				while (true) {

					ret = generic("CHARACTER OPTIONS", setupCharacterOptions, 5, suboption);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					switch (suboption) {

						case 0: // Character name

							if (textInput("character name:", setup.characterName) == E_QUIT) return E_QUIT;

							break;

						default: // Character colour

							subsuboption = 0;
							ret = generic(nullptr, setupCharacterColOptions, 8, subsuboption);

							if (ret == E_QUIT) return E_QUIT;

							if (ret == E_NONE)
								setup.characterCols[suboption - 1] = setupCharacterCols[subsuboption];

							break;

					}

				}

				break;

			case 1:

#ifdef NO_KEYBOARD_CFG
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
#else
				if (setupKeyboard() == E_QUIT) return E_QUIT;
#endif

				break;

			case 2:

#ifdef NO_CONTROLLER_CFG
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
#else
				if (setupJoystick() == E_QUIT) return E_QUIT;
#endif

				break;

			case 3:

#if defined(NO_RESIZE) && !defined(SCALE)
				if (message("FEATURE NOT AVAILABLE") == E_QUIT) return E_QUIT;
#else
				if (setupVideo() == E_QUIT) return E_QUIT;
#endif

				break;

			case 4:

				if (setupAudio() == E_QUIT) return E_QUIT;

				break;

			case 5:

				suboption = 0;

				while (true) {

					ret = generic("GAME OPTIONS", setupMods, 3, suboption);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					if (setupMods[suboption] == setupModsOff[suboption])
						setupMods[suboption] = setupModsOn[suboption];
					else
						setupMods[suboption] = setupModsOff[suboption];

					setup.slowMotion = (setupMods[0] == setupModsOn[0]);
					setup.leaveUnneeded = (setupMods[1] == setupModsOn[1]);
					setup.manyBirds = (setupMods[2] == setupModsOn[2]);

				}

				break;

		}

	}

	return E_NONE;

}
