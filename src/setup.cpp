
/**
 *
 * @file setup.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd of August 2005: Created main.c and menu.c
 * - 3rd of February 2009: Renamed main.c to main.cpp and menu.c to menu.cpp
 * - 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * - 26th July 2009: Renamed menusetup.cpp to setupmenu.cpp
 * - 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the running of setup menus.
 *
 */


#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/player.h"
#include "setup.h"
#include "util.h"
#include "io/log.h"

#ifdef __SYMBIAN32__
    #ifdef UIQ3
        #define CONFIG_FILE "c:\\shared\\openjazz\\openjazz.cfg"
    #else
        #define CONFIG_FILE "c:\\data\\openjazz\\openjazz.cfg"
    #endif
#elif defined(__riscos__)
    #define CONFIG_FILE "/<Choices$Write>/OpenJazz/openjazz.cfg"
#elif __vita__
    #define CONFIG_FILE "ux0:data/jazz/openjazz.cfg"
#else
    #define CONFIG_FILE "openjazz.cfg"
#endif


/**
 * Create default setup
 */
Setup::Setup () {

	// Create the player's name
	characterName = createEditableString(CHAR_NAME);

	// Assign the player's colour
	characterCols[0] = CHAR_FUR;
	characterCols[1] = CHAR_BAND;
	characterCols[2] = CHAR_GUN;
	characterCols[3] = CHAR_WBAND;

	// scale2x by default
	scale2x = true;

	return;

}


/**
 * Delete the setup data
 */
Setup::~Setup () {

	delete[] characterName;

}


/**
 * Load settings from config file.
 */
void Setup::load (int* videoW, int* videoH, bool* fullscreen, int* videoScale) {

	File* file;
	int count;

	// Open config file

	try {

		file = new File(CONFIG_FILE, false);

	} catch (int e) {

		LOG_DEBUG("Configuration file not found.");

		return;

	}

	// Check that the config file has the correct version
	if (file->loadChar() != 6) {

		LOG_WARN("Valid configuration file not found.");
		delete file;

		return;

	}


	// Read video settings
	*videoW = file->loadShort(MAX_SCREEN_WIDTH);
	*videoH = file->loadShort(MAX_SCREEN_HEIGHT);
	count = file->loadChar();
#ifndef FULLSCREEN_ONLY
	*fullscreen = count & 1;
#endif
#ifdef SCALE
	if (count >= 10) count = 2;
	*videoScale = count >> 1;
#endif


	// Read controls
	for (count = 0; count < CONTROLS - 4; count++)
		controls.setKey(count, (SDLKey)(file->loadInt()));

	for (count = 0; count < CONTROLS; count++)
		controls.setButton(count, file->loadInt());

	for (count = 0; count < CONTROLS; count++) {

		int a, d;

		a = file->loadInt();
		d = file->loadInt();
		controls.setAxis(count, a, d);

	}

	for (count = 0; count < CONTROLS; count++) {

		int h, d;

		h = file->loadInt();
		d = file->loadInt();
		controls.setHat(count, h, d);

	}

	// Read the player's name
	for (count = 0; count < STRING_LENGTH; count++)
		setup.characterName[count] = file->loadChar();

	setup.characterName[STRING_LENGTH] = 0;

	// Read the player's colours
	setup.characterCols[0] = file->loadChar();
	setup.characterCols[1] = file->loadChar();
	setup.characterCols[2] = file->loadChar();
	setup.characterCols[3] = file->loadChar();

	// Read the music and sound effect volume
	setMusicVolume(file->loadChar());
	setSoundVolume(file->loadChar());

	// Read gameplay options
	count = file->loadChar();
	setup.manyBirds = ((count & 1) != 0);
	setup.leaveUnneeded = ((count & 2) != 0);
	setup.slowMotion = ((count & 4) != 0);
	setup.scale2x = ((count & 8) == 0);


	delete file;


	return;

}


/**
 * Save settings to config file.
 */
void Setup::save () {

	File *file;
	int count;
	int videoScale;

	// Open config file
	try {

		file = new File(CONFIG_FILE, true);

	} catch (int e) {

		file = NULL;

	}

	// Check that the config file was opened
	if (!file) {

		LOG_ERROR("Could not write configuration file: File could not be opened.");

		return;

	}


	// Write the version number
	file->storeChar(6);

	// Write video settings
	file->storeShort(video.getWidth());
	file->storeShort(video.getHeight());
#ifdef SCALE
	videoScale = video.getScaleFactor();
#else
	videoScale = 1;
#endif
	videoScale <<= 1;
#ifndef FULLSCREEN_ONLY
	videoScale |= video.isFullscreen()? 1: 0;
#endif
	file->storeChar(videoScale);


	// Write controls
	for (count = 0; count < CONTROLS - 4; count++)
		file->storeInt(controls.getKey(count));

	for (count = 0; count < CONTROLS; count++)
		file->storeInt(controls.getButton(count));

	for (count = 0; count < CONTROLS; count++) {

		file->storeInt(controls.getAxis(count));
		file->storeInt(controls.getAxisDirection(count));

	}

	for (count = 0; count < CONTROLS; count++) {

		file->storeInt(controls.getHat(count));
		file->storeInt(controls.getHatDirection(count));

	}

	// Write the player's name
	for (count = 0; count < STRING_LENGTH; count++)
		file->storeChar(setup.characterName[count]);

	// Write the player's colour
	file->storeChar(setup.characterCols[0]);
	file->storeChar(setup.characterCols[1]);
	file->storeChar(setup.characterCols[2]);
	file->storeChar(setup.characterCols[3]);

	// Write the music and sound effect volume
	file->storeChar(getMusicVolume());
	file->storeChar(getSoundVolume());

	// Write gameplay options

	count = 0;

	if (setup.manyBirds) count |= 1;
	if (setup.leaveUnneeded) count |= 2;
	if (setup.slowMotion) count |= 4;
	if (!setup.scale2x) count |= 8;

	file->storeChar(count);


	delete file;


	return;

}

