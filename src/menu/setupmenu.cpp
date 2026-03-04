
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
	int progress, x, y;

	progress = 0;

	while (true) {

		int character = loop(SET_KEY_LOOP);

		if (character == E_QUIT) return E_QUIT;

		if (character == controls.getKey(C_ESCAPE)) return E_NONE;

		// Allow gamepad back/escape button to exit
		// (loop(SET_KEY_LOOP) only returns keyboard events; use release() for gamepad)
		if (controls.release(C_ESCAPE)) return E_NONE;

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

		fontmn2->showString("KEYBOARD CONFIGURATION", canvasW >> 1,
			(canvasH >> 1) - 80, alignX::Center);

		for (int count = 0; count < PCONTROLS; count++) {

			if (count < progress)
				fontmn2->showString("okay", (canvasW >> 1) + 16,
					(canvasH >> 1) + (count << 4) - 56);

			else if (count == progress) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(options[count], (canvasW >> 1) - 96,
				(canvasH >> 1) + (count << 4) - 56);

			if (count == progress) {

				fontmn2->showString("press key", canvasW >> 1,
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
 * Format a control's current gamepad binding as a short human-readable string.
 * Returns e.g. "btn 0", "ax5+", "hat up", or "none".
 */
static void formatBinding (int control, char *buf, int bufLen) {
	int btn = controls.getButton(control);
	int altBtn = controls.getAltButton(control);
	int axis = controls.getAxis(control);
	int axDir = controls.getAxisDirection(control);
	int hat = controls.getHat(control);

	// Primary binding
	if (btn >= 0) {
		if (altBtn >= 0)
			SDL_snprintf(buf, bufLen, "btn%d/btn%d", btn, altBtn);
		else
			SDL_snprintf(buf, bufLen, "btn %d", btn);
	} else if (axis >= 0) {
		SDL_snprintf(buf, bufLen, "ax%d%c", axis, axDir ? '+' : '-');
	} else if (hat >= 0) {
		const char *dirs[] = {"up","right","down","left"};
		int d = hat & 0xF;
		int di = (d == SDL_HAT_UP) ? 0 : (d == SDL_HAT_RIGHT) ? 1 :
		         (d == SDL_HAT_DOWN) ? 2 : 3;
		SDL_snprintf(buf, bufLen, "hat%s", dirs[di]);
	} else {
		SDL_snprintf(buf, bufLen, "none");
	}
}


/**
 * Apply the "original" button preset (classic JJ1 layout):
 * B=jump, A=fire, Y=weapon, RB=menu, LB=pause, no alt bindings.
 */
static void applyPresetOriginal () {
	controls.setButton(C_JUMP,   1);  // B
	controls.setButton(C_SWIM,   1);  // B
	controls.setButton(C_FIRE,   0);  // A
	controls.setButton(C_CHANGE, 3);  // Y
	controls.setButton(C_ENTER,  0);  // A
	controls.setButton(C_ESCAPE, 1);  // B = back
	controls.setButton(C_MENU,   7);  // Start = menu
	controls.setButton(C_PAUSE,  4);  // LB
	for (int i = 0; i < CONTROLS; i++) controls.setAltButton(i, -1);
	controls.setAxis(C_FIRE, -1, true);
}


/**
 * Apply the "Jazz² style" button preset (matches jazz2-native layout):
 * A=jump, X=fire, B=back (alt escape), Start=menu, Select=pause, RT=alt fire.
 */
static void applyPresetJazz2 () {
	controls.setButton(C_JUMP,   0);  // A
	controls.setButton(C_SWIM,   0);  // A
	controls.setButton(C_FIRE,   2);  // X
	controls.setButton(C_CHANGE, 3);  // Y
	controls.setButton(C_ENTER,  0);  // A
	controls.setButton(C_ESCAPE, 1);  // B = back
	controls.setButton(C_MENU,   7);  // Start = menu
	controls.setButton(C_PAUSE,  6);  // Select
	for (int i = 0; i < CONTROLS; i++) controls.setAltButton(i, -1);
	controls.setAxis(C_FIRE, 5, true);   // RT = alt fire
}


/**
 * Run the gamepad configuration menu.
 * Shows all player controls with their current bindings in a list.
 * Navigate with up/down, press Enter/A to reassign a control.
 * Preset buttons quickly apply common layouts.
 *
 * @return Error code
 */
int SetupMenu::setupJoystick () {

	// Controls shown in the list (indices into the controls arrays)
	static const int ctrlIndex[] = {
		C_UP, C_DOWN, C_LEFT, C_RIGHT,
		C_JUMP, C_SWIM, C_FIRE, C_CHANGE,
		C_ENTER, C_ESCAPE, C_MENU, C_PAUSE
	};
	static const char *ctrlName[] = {
		"up", "down", "left", "right",
		"jump", "swim", "fire", "weapon",
		"confirm", "back", "menu", "pause"
	};
	static const int nCtrl = 12;

	// Two extra rows: preset buttons
	static const int nRows = nCtrl + 2;

	int selection = 0;
	bool assigning = false;
	int x, y;
	char bindBuf[32];

	while (true) {

		int control = loop(assigning ? SET_JOYSTICK_LOOP : NORMAL_LOOP);

		if (control == E_QUIT) return E_QUIT;

		if (assigning) {
			// Waiting for any joystick input to assign to the current control
			int ctrl = ctrlIndex[selection];
			bool assigned = false;

			switch (control & 0xF00) {
				case JOYSTICKB:
					controls.setButton(ctrl, control & 0xFF);
					assigned = true;
					break;
				case JOYSTICKANEG:
					controls.setAxis(ctrl, control & 0xFF, false);
					assigned = true;
					break;
				case JOYSTICKAPOS:
					controls.setAxis(ctrl, control & 0xFF, true);
					assigned = true;
					break;
				case JOYSTICKHUP:
					controls.setHat(ctrl, control & 0xFF, SDL_HAT_UP);
					assigned = true;
					break;
				case JOYSTICKHLFT:
					controls.setHat(ctrl, control & 0xFF, SDL_HAT_LEFT);
					assigned = true;
					break;
				case JOYSTICKHRHT:
					controls.setHat(ctrl, control & 0xFF, SDL_HAT_RIGHT);
					assigned = true;
					break;
				case JOYSTICKHDWN:
					controls.setHat(ctrl, control & 0xFF, SDL_HAT_DOWN);
					assigned = true;
					break;
			}

			if (assigned) {
				playConfirmSound();
				assigning = false;
			}

			// Escape key cancels assignment
			if (controls.release(C_ESCAPE)) {
				assigning = false;
			}

		} else {
			// Navigation mode
			if (controls.release(C_ESCAPE)) return E_NONE;

			if (controls.getCursor(x, y) &&
				(x < 100) && (y >= canvasH - 12) &&
				controls.wasCursorReleased()) return E_NONE;

			if (controls.release(C_UP)) {
				if (selection > 0) selection--;
				else selection = nRows - 1;
			}
			if (controls.release(C_DOWN)) {
				if (selection < nRows - 1) selection++;
				else selection = 0;
			}

			if (controls.release(C_ENTER)) {
				playConfirmSound();
				if (selection < nCtrl) {
					// Start waiting for joystick input for this control
					assigning = true;
				} else if (selection == nCtrl) {
					applyPresetOriginal();
				} else {
					applyPresetJazz2();
				}
			}
		}


		SDL_Delay(T_MENU_FRAME);
		video.clearScreen(0);

		fontmn2->showString("GAMEPAD CONFIGURATION", canvasW >> 1,
			(canvasH >> 1) - 80, alignX::Center);

		int listTop = (canvasH >> 1) - 56;

		for (int i = 0; i < nCtrl; i++) {
			int rowY = listTop + i * 11;

			if (i == selection) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(ctrlName[i], (canvasW >> 1) - 96, rowY);

			if (i == selection && assigning) {
				fontmn2->showString("press control", (canvasW >> 1) + 8, rowY);
			} else {
				formatBinding(ctrlIndex[i], bindBuf, sizeof(bindBuf));
				fontmn2->showString(bindBuf, (canvasW >> 1) + 8, rowY);
			}

			if (i == selection) fontmn2->restorePalette();
		}

		// Preset buttons
		int presetY = listTop + nCtrl * 11 + 6;

		if (selection == nCtrl) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("preset: original", (canvasW >> 1) - 96, presetY);
		if (selection == nCtrl) fontmn2->restorePalette();

		if (selection == nCtrl + 1) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("preset: jazz2", (canvasW >> 1) - 96, presetY + 11);
		if (selection == nCtrl + 1) fontmn2->restorePalette();

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
	// Aspect ratio presets: 0=free, 1=16:9, 2=16:10 (Steam Deck), 3=4:3
	const char *aspectString[4] = { "free", "16:9", "16:10", "4:3" };
	int scaleFactor = video.getScaleFactor();
	scalerType scaleMethod = video.getScaleMethod();
	int screenW, screenH, oldscreenW, oldscreenH, x, y;
	screenW = oldscreenW = video.getWidth();
	screenH = oldscreenH = video.getHeight();
	int selection = 0;
	int aspectIndex = 0;
	bool resOK = true;

	char scaleString[3] = "Yx";
	bool fillScaleString = true;
	char resString[22] = "game res: XXXX x YYYY";
	bool fillResString = true;
	int resStringWidth = 0;

	// helpers
	auto calcAspectHeight = [&] (int w) -> int {
		switch (aspectIndex) {
			case 1: return (w * 9)  / 16;  // 16:9
			case 2: return (w * 10) / 16;  // 16:10
			case 3: return (w * 3)  / 4;   // 4:3
			default: return 0;
		}
	};

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
		// Apply aspect ratio constraint when locked
		if (aspectIndex > 0) {
			int newH = calcAspectHeight(screenW);
			screenH = CLAMP(newH, video.getMinHeight(), video.getMaxHeight());
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
	auto changeAspect = [&] (bool isPositive) {
		if (isPositive && aspectIndex < 3)
			aspectIndex++;
		else if (!isPositive && aspectIndex > 0)
			aspectIndex--;
		// Apply newly selected aspect ratio to current width
		if (aspectIndex > 0) {
			int newH = calcAspectHeight(screenW);
			screenH = CLAMP(newH, video.getMinHeight(), video.getMaxHeight());
		}
	};

	auto changeScaleMethod = [&] (bool isPositive) {
		// TODO
#if 0
		if (isPositive && scaleMethod != scalerType::hqx)
			scaleMethod = static_cast<scalerType>(+scaleMethod + 1);
		else if(!isPositive && scaleMethod != scalerType::None)
			scaleMethod = static_cast<scalerType>(+scaleMethod - 1);
#else
	OJ_UNUSED(isPositive);

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

			// ESC
			if ((x >= 32) && (x < 132) && (y >= canvasH - 12)
				&& controls.wasCursorReleased())
				return E_NONE;

			// TODO: selection? not feasible with one button
		}

		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);


		// Show screen corners
		video.drawRect(0, 0, 32, 32, 79);
		video.drawRect(canvasW - 32, 0, 32, 32, 79);
		video.drawRect(canvasW - 32, canvasH - 32, 32, 32, 79);
		video.drawRect(0, canvasH - 32, 32, 32, 79);

		fontmn2->showString("VIDEO OPTIONS", canvasW >> 1, (canvasH >> 1) - 80, alignX::Center);

		// Game Resolution
		if(fillResString) {
			snprintf(resString, sizeof(resString), "game res: %d x %d", canvasW, canvasH);
			resStringWidth = fontmn2->getStringWidth(resString);
			fillResString = false;
		}
		video.drawRect(((canvasW - resStringWidth) >> 1) - 4, (canvasH >> 1) - 50, resStringWidth + 4, 14, 46);
		fontmn2->showString(resString, (canvasW - resStringWidth) >> 1, (canvasH >> 1) - 48);

		switchPalette(selection == 0);

		// Width
		fontmn2->showString("width:", (canvasW >> 1) - 80, (canvasH >> 1) - 16);
		fontmn2->showNumber(screenW, (canvasW >> 1) + 40, (canvasH >> 1) - 16);

		switchPalette(selection == 1);

		// Height
		fontmn2->showString("height:", (canvasW >> 1) - 80, canvasH >> 1);
		fontmn2->showNumber(screenH, (canvasW >> 1) + 40, canvasH >> 1);

		switchPalette(selection == 2);

		// Factor
		if(fillScaleString) {
			snprintf(scaleString, sizeof(scaleString), "%dx", scaleFactor);
			fillScaleString = false;
		}
		fontmn2->showString("scale:", (canvasW >> 1) - 80, (canvasH >> 1) + 16);
		fontmn2->showString(scaleString, (canvasW >> 1) + 8, (canvasH >> 1) + 16);

		switchPalette(selection == 3);

		// Method
		fontmn2->showString("method:", (canvasW >> 1) - 80, (canvasH >> 1) + 32);
		fontmn2->showString(methodString[+scaleMethod], (canvasW >> 1) + 8, (canvasH >> 1) + 32);

		switchPalette(selection == 4);

		// Aspect ratio
		fontmn2->showString("aspect:", (canvasW >> 1) - 80, (canvasH >> 1) + 48);
		fontmn2->showString(aspectString[aspectIndex], (canvasW >> 1) + 8, (canvasH >> 1) + 48);

		switchPalette(false);

		if (controls.release(C_UP)) {
			if(selection > 0)
				selection--;
			else
				selection = 4;
		}

		if (controls.release(C_DOWN)) {
			if(selection < 4)
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
			case 4:
				changeAspect(hasPressedRight);
				break;
			}
			resOK = true;
		}

		// Check for a resolution or scaling change
		if (screenH != oldscreenH || screenW != oldscreenW ||
			scaleFactor != video.getScaleFactor() || scaleMethod != video.getScaleMethod()) {

			fontmn2->showString(resOK ? "press enter to apply" : "invalid resolution!",
				(canvasW >> 1), (canvasH >> 1) + 64, alignX::Center);

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

		// cannot use showEscString() here, needs shifting to the right
		fontbig->showString(ESCAPE_STRING, 35, canvasH, alignX::Left, alignY::Bottom);
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

			// ESC
			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_NONE;

			// start from drawing position
			x -= (canvasW >> 1) + 48;
			y -= canvasH >> 1;

			if ((x >= 0) && (x < (MAX_VOLUME >> 1)) && (y >= 0) && (y < 11)) setMusicVolume(x << 1);
			if ((x >= 0) && (x < (MAX_VOLUME >> 1)) && (y >= 16) && (y < 27)) setSoundVolume(x << 1);

			if (controls.wasCursorReleased()) playConfirmSound();

		}

		SDL_Delay(T_MENU_FRAME);

		video.clearScreen(0);

		fontmn2->showString("AUDIO OPTIONS", canvasW >> 1, (canvasH >> 1) - 80, alignX::Center);

		// Music Volume
		if (!soundActive) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("music volume", (canvasW >> 1) - 88, canvasH >> 1);
		fontmn2->restorePalette();

		video.drawRect((canvasW >> 1) + 48, canvasH >> 1, getMusicVolume() >> 1, 11, 175);

		// Sound Volume
		if (soundActive) fontmn2->mapPalette(240, 8, 114, 16);
		fontmn2->showString("effect volume", (canvasW >> 1) - 88, (canvasH >> 1) + 16);
		fontmn2->restorePalette();

		video.drawRect((canvasW >> 1) + 48, (canvasH >> 1) + 16, getSoundVolume() >> 1, 11, 175);

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

	const char* setupOptions[6] = {"character", "keyboard", "gamepad", "video", "audio", "gameplay"};
	const char* setupCharacterOptions[5] = {"name", "fur", "bandana", "gun", "wristband"};
	const char* setupCharacterColOptions[8] = {"white", "red", "orange", "yellow", "green", "blue", "animation 1", "animation 2"};
	const unsigned char setupCharacterCols[8] = {PC_GREY, PC_RED, PC_ORANGE, PC_YELLOW, PC_LGREEN, PC_BLUE, PC_SANIM, PC_LANIM};
	const char* setupModsOff[5] = {"slow motion: off", "extra items: take", "bird limit: one", "hud style: classic", "hud panel: float"};
	const char* setupModsOn[5] = {"slow motion: on", "extra items: leave", "bird limit: no", "hud style: old fps", "hud panel: fixed"};
	const char* setupMods[5];
	int ret;
	int option, suboption, subsuboption;

	option = 0;

	setupMods[0] = (setup.slowMotion? setupModsOn[0]: setupModsOff[0]);
	setupMods[1] = (setup.leaveUnneeded? setupModsOn[1]: setupModsOff[1]);
	setupMods[2] = (setup.manyBirds? setupModsOn[2]: setupModsOff[2]);
	setupMods[3] = (setup.hudStyle == hudType::FPS? setupModsOn[3]: setupModsOff[3]);
	setupMods[4] = (setup.hudFixed? setupModsOn[4]: setupModsOff[4]);

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

				// Pre-compute max pixel width from all Off+On strings so the
				// menu stays centered when options are toggled.
				{
					int modsMaxWidth = 0;
					for (int i = 0; i < 5; i++) {
						int w1 = fontmn2->getStringWidth(setupModsOff[i]);
						int w2 = fontmn2->getStringWidth(setupModsOn[i]);
						if (w1 > modsMaxWidth) modsMaxWidth = w1;
						if (w2 > modsMaxWidth) modsMaxWidth = w2;
					}

				while (true) {

					ret = generic("GAME OPTIONS", setupMods, 5, suboption, modsMaxWidth);

					if (ret == E_QUIT) return E_QUIT;
					if (ret < 0) break;

					if (setupMods[suboption] == setupModsOff[suboption])
						setupMods[suboption] = setupModsOn[suboption];
					else
						setupMods[suboption] = setupModsOff[suboption];

					setup.slowMotion = (setupMods[0] == setupModsOn[0]);
					setup.leaveUnneeded = (setupMods[1] == setupModsOn[1]);
					setup.manyBirds = (setupMods[2] == setupModsOn[2]);
					if(setupMods[3] == setupModsOn[3]) {
						setup.hudStyle = hudType::FPS;
					} else {
						setup.hudStyle = hudType::Classic;
					}
					setup.hudFixed = (setupMods[4] == setupModsOn[4]);

				}

				} // modsMaxWidth scope

				break;

		}

	}

	return E_NONE;

}
