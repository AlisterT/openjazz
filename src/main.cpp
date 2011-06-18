
/**
 *
 * @file main.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed main.c to main.cpp
 * 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * 13th July 2009: Created controls.cpp from parts of main.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2011 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Contains the main function.
 *
 */


#define EXTERN

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/video.h"
#include "io/network.h"
#include "io/sound.h"
#include "jj2level/jj2level.h"
#include "level/level.h"
#include "menu/menu.h"
#include "player/player.h"
#include "scene/scene.h"
#include "loop.h"
#include "util.h"

#include <string.h>

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	#include "platforms/wiz.h"
#endif

#ifdef __SYMBIAN32__
extern char KOpenJazzPath[256];
extern float sinf (float);
#else
	#include <math.h>
#endif

#define PI 3.141592f


/**
 * Initialises OpenJazz.
 *
 * Establishes the paths from which to read files, loads configuration, sets up
 * the game window and loads required data.
 *
 * @param argc Number of arguments, as passed to main function
 * @param argv Array of argument strings, as apsse to main function
 *
 * @return Error code
 */
int loadMain (int argc, char *argv[]) {

	File* file;
	unsigned char* pixels = NULL;
	int count;
	int screenW = SW;
	int screenH = SH;
	int scaleFactor = 1;
#ifdef FULLSCREEN_ONLY
	bool fullscreen = true;
#else
	bool fullscreen = false;
#endif


	// Determine paths

	// Use hard-coded paths, if available

#ifdef DATAPATH
	firstPath = new Path(NULL, createString(DATAPATH));
#else
	firstPath = NULL;
#endif

#ifdef __SYMBIAN32__
	#ifdef UIQ3
	firstPath = new Path(firstPath, createString("c:\\shared\\openjazz\\"));
	#else
	firstPath = new Path(firstPath, createString("c:\\data\\openjazz\\"));
	#endif
	firstPath = new Path(firstPath, createString(KOpenJazzPath));
#endif


	// Use any provided paths, appending a directory separator as necessary

	for (count = 1; count < argc; count++) {

		// If it isn't an option, it should be a path
		if (argv[count][0] != '-') {

#ifdef WIN32
			if (argv[count][strlen(argv[count]) - 1] != '\\') {

				firstPath = new Path(firstPath, createString(argv[count], "\\"));
#else
			if (argv[count][strlen(argv[count]) - 1] != '/') {

				firstPath = new Path(firstPath, createString(argv[count], "/"));
#endif

			} else {

				firstPath = new Path(firstPath, createString(argv[count]));

			}

		}

	}


	// Use the path of the program

	count = strlen(argv[0]) - 1;

	// Search for directory separator
#ifdef WIN32
	while ((argv[0][count] != '\\') && (count >= 0)) count--;
#else
	while ((argv[0][count] != '/') && (count >= 0)) count--;
#endif

	// If a directory was found, copy it to the path
	if (count > 0) {

		firstPath = new Path(firstPath, new char[count + 2]);
		memcpy(firstPath->path, argv[0], count + 1);
		firstPath->path[count + 1] = 0;

	}


	// Use the user's home directory, if available

#ifdef HOMEDIR
	#ifdef WIN32
	firstPath = new Path(firstPath, createString(getenv("HOME"), "\\"));
	#else
	firstPath = new Path(firstPath, createString(getenv("HOME"), "/."));
	#endif
#endif


	// Use the current working directory

	firstPath = new Path(firstPath, createString(""));



	// Default settings

	// Sound settings
#if defined(WIZ) || defined(GP2X)
	volume = 40;
#endif
	soundsVolume = MAX_VOLUME >> 2;


	// Create the network address
	netAddress = createString(NET_ADDRESS);


	// Open config file

	try {

		file = new File(CONFIG_FILE, false);

	} catch (int e) {

		file = NULL;

	}

	// Check that the config file was opened, and has the correct version
	if (file && (file->loadChar() == 3)) {

		// Read video settings
		screenW = file->loadShort(7680);
		screenH = file->loadShort(4800);

		scaleFactor = file->loadChar();
#ifndef FULLSCREEN_ONLY
		fullscreen = scaleFactor & 1;
#endif
#ifdef SCALE
		scaleFactor >>= 1;
		if (scaleFactor > 4) scaleFactor = 1;
#endif


		// Read controls
		for (count = 0; count < CONTROLS - 4; count++)
			controls.setKey(count, (SDLKey)(file->loadInt()));

		for (count = 0; count < CONTROLS; count++)
			controls.setButton(count, file->loadInt());

		for (count = 0; count < CONTROLS; count++)
			controls.setAxis(count, file->loadInt(), file->loadInt());

		// Read the player's name
		for (count = 0; count < STRING_LENGTH; count++)
			setup.characterName[count] = file->loadChar();

		setup.characterName[STRING_LENGTH] = 0;

		// Read the player's colours
		setup.characterCols[0] = file->loadChar();
		setup.characterCols[1] = file->loadChar();
		setup.characterCols[2] = file->loadChar();
		setup.characterCols[3] = file->loadChar();

		// Read the sound effect volume
		soundsVolume = file->loadChar();
		if (soundsVolume > MAX_VOLUME) soundsVolume = MAX_VOLUME;

		// Read gameplay options
		count = file->loadChar();
		setup.manyBirds = ((count & 1) != 0);
		setup.leaveUnneeded = ((count & 2) != 0);


		delete file;

	} else {

		log("Valid configuration file not found.");

	}


	// Get command-line override

#ifndef FULLSCREEN_ONLY
	for (count = 1; count < argc; count++) {

		// If there's a hyphen, it should be an option
		if (argv[count][0] == '-') {

			if (argv[count][1] == 'f') fullscreen = true;

		}

	}
#endif


	// Create the game's window

	canvas = NULL;

	if (!video.init(screenW, screenH, fullscreen)) {

		delete firstPath;

		return E_VIDEO;

	}

#ifdef SCALE
	video.setScaleFactor(scaleFactor);
#endif


	if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);


	// Set up audio
	openAudio();



	// Load fonts

	// Open the panel, which contains two fonts

	try {

		file = new File(F_PANEL, false);

	} catch (int e) {

		closeAudio();

		delete firstPath;

		return e;

	}

	pixels = file->loadRLE(46272);

	delete file;

	panelBigFont = NULL;
	panelSmallFont = NULL;
	font2 = NULL;
	fontbig = NULL;
	fontiny = NULL;
	fontmn1 = NULL;

	try {

		panelBigFont = new Font(pixels + (40 * 320), true);
		panelSmallFont = new Font(pixels + (48 * 320), false);
		font2 = new Font(F_FONT2_0FN);
		fontbig = new Font(F_FONTBIG_0FN);
		fontiny = new Font(F_FONTINY_0FN);
		fontmn1 = new Font(F_FONTMN1_0FN);
		fontmn2 = new Font(F_FONTMN2_0FN);

	} catch (int e) {

		if (panelBigFont) delete panelBigFont;
		if (panelSmallFont) delete panelSmallFont;
		if (font2) delete font2;
		if (fontbig) delete fontbig;
		if (fontiny) delete fontiny;
		if (fontmn1) delete fontmn1;

		delete[] pixels;

		closeAudio();

		delete firstPath;

		return e;

	}

	delete[] pixels;


	// Establish arbitrary timing
	globalTicks = SDL_GetTicks() - 20;


	// Fill trigonomatric function look-up tables
	for (count = 0; count < 1024; count++)
		sinLut[count] = fixed(sinf(2 * PI * float(count) / 1024.0f) * 1024.0f);


	// Initiate networking
	net = new Network();


	baseLevel = NULL;
	level = NULL;
	jj2Level = NULL;


	return E_NONE;

}


/**
 * De-initialises OpenJazz.
 *
 * Frees data, writes configuration, and shuts down SDL.
 */
void freeMain () {

	File *file;
	int count;
	int scaleFactor;

	delete net;

	delete panelBigFont;
	delete panelSmallFont;
	delete font2;
	delete fontbig;
	delete fontiny;
	delete fontmn1;
	delete fontmn2;

#ifdef SCALE
	scaleFactor = video.getScaleFactor();

	if (scaleFactor > 1) SDL_FreeSurface(canvas);
#else
	scaleFactor = 1;
#endif

	closeAudio();


	// Open config file
	try {

		file = new File(CONFIG_FILE, true);

	} catch (int e) {

		file = NULL;

	}

	// Check that the config file was opened
	if (file) {

		// Write the version number
		file->storeChar(3);

		// Write video settings
		file->storeShort(video.getWidth());
		file->storeShort(video.getHeight());
		scaleFactor <<= 1;
#ifndef FULLSCREEN_ONLY
		scaleFactor |= video.isFullscreen()? 1: 0;
#endif
		file->storeChar(scaleFactor);


		// Write controls
		for (count = 0; count < CONTROLS - 4; count++)
			file->storeInt(controls.getKey(count));

		for (count = 0; count < CONTROLS; count++)
			file->storeInt(controls.getButton(count));

		for (count = 0; count < CONTROLS; count++) {

			file->storeInt(controls.getAxis(count));
			file->storeInt(controls.getAxisDirection(count));

		}

		// Write the player's name
		for (count = 0; count < STRING_LENGTH; count++)
			file->storeChar(setup.characterName[count]);

		// Write the player's colour
		file->storeChar(setup.characterCols[0]);
		file->storeChar(setup.characterCols[1]);
		file->storeChar(setup.characterCols[2]);
		file->storeChar(setup.characterCols[3]);

		// Write the sound effect volume
		file->storeChar(soundsVolume);

		// Write gameplay options

		count = 0;

		if (setup.manyBirds) count |= 1;
		if (setup.leaveUnneeded) count |= 2;

		file->storeChar(count);


		delete file;

	} else {

		logError("Could not write configuration file",
			"File could not be opened.");

	}

	delete firstPath;


	SDL_Quit();


	return;

}


/**
 * Process iteration.
 *
 * Called once per game iteration. Updates timing, video, and input
 *
 * @param type Type of loop. Normal, typing, or input configuration
 * @param paletteEffects Palette effects to apply to video output
 *
 * @return Error code
 */
int loop (LoopType type, PaletteEffect* paletteEffects) {

	SDL_Event event;
	int prevTicks, ret;


	// Update tick count
	prevTicks = globalTicks;
	globalTicks = SDL_GetTicks();

	// Show what has been drawn
	video.flip(globalTicks - prevTicks, paletteEffects);


	// Process system events
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT) return E_QUIT;

		ret = controls.update(&event, type);

		if (ret != E_NONE) return ret;

		video.update(&event);

#if defined(WIZ) || defined(GP2X)
		if ((event.type == SDL_JOYBUTTONDOWN) ||
			(event.type == SDL_JOYBUTTONUP)) {

				if (event.jbutton.button ==  GP2X_BUTTON_VOLUP ) {
					if( event.type == SDL_JOYBUTTONDOWN )
						volume_direction = VOLUME_UP;
					else
						volume_direction = VOLUME_NOCHG;
				}
				if (event.jbutton.button ==  GP2X_BUTTON_VOLDOWN ) {
					if( event.type == SDL_JOYBUTTONDOWN )
						volume_direction = VOLUME_DOWN;
					else
						volume_direction = VOLUME_NOCHG;
				}

		}
#endif

	}

	controls.loop();


#if defined(WIZ) || defined(GP2X)
	WIZ_AdjustVolume( volume_direction );
#endif

	return E_NONE;

}


/**
 * Main.
 *
 * Initialises SDL and launches game.
 */
int main(int argc, char *argv[]) {

	MainMenu *mainMenu = NULL;
	Scene *scene = NULL;

	// Initialise SDL

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {

		logError("Could not start SDL", SDL_GetError());

		return -1;

	}


	// Load universal game data and establish a window

	if (loadMain(argc, argv) != E_NONE) {

		SDL_Quit();

		return -1;

	}


	// Show the startup cutscene

	try {

		scene = new Scene(F_STARTUP_0SC);

	} catch (int e) {

		freeMain();

		return e;

	}

	if (scene->play() == E_QUIT) {

		delete scene;
		freeMain();

		return 0;

	}

	delete scene;


	// Load the menu
	try {

		mainMenu = new MainMenu();

	} catch (int e) {

		freeMain();

		return e;

	}

	// Run the main menu
	if (mainMenu->main() != E_QUIT) {

		// Show the ending cutscene

		try {

			scene = new Scene(F_END_0SC);

		} catch (int e) {

			delete mainMenu;
			freeMain();

			return e;

		}

		scene->play();

		delete scene;


	}

	delete mainMenu;
	freeMain();


	return 0;

}


