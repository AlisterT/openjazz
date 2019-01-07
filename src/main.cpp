
/**
 *
 * @file main.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed main.c to main.cpp
 * - 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * - 13th July 2009: Created controls.cpp from parts of main.cpp
 * - 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @par Description:
 * Contains the main function.
 *
 */


#define EXTERN

#include "game/game.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/network.h"
#include "io/sound.h"
#include "jj2level/jj2level.h"
#include "jj1level/jj1level.h"
#include "menu/menu.h"
#include "player/player.h"
#include "jj1scene/jj1scene.h"
#include "loop.h"
#include "setup.h"
#include "util.h"

#ifdef PSP
	#include <pspsdk.h>
	#include <pspkernel.h>
	#include <pspthreadman.h>
	#include <pspmoduleinfo.h>
	#include <pspdebug.h>
	#include <psputility.h>
	#include <pspdisplay.h>
#elif defined(_3DS)
	#include <3ds.h>
#elif defined(WII)
	#include <unistd.h>
	#include <fat.h>
	#include "platforms/wii.h"
#elif defined(__HAIKU__)
	#include <Alert.h>
	#include <FindDirectory.h>
	#include <fs_info.h>
#endif

#include <string.h>

#if defined(WIZ) || defined(GP2X)
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
 * @param argv Array of argument strings, as passed to main function
 */
void startUp (int argc, char *argv[]) {

	File* file;
	unsigned char* pixels = NULL;
	int count;
	int screenW = DEFAULT_SCREEN_WIDTH;
	int screenH = DEFAULT_SCREEN_HEIGHT;
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

#ifdef __HAIKU__
	dev_t volume = dev_for_path("/boot");
	char buffer[10 + B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH];
	status_t result;

	result = find_directory(B_SYSTEM_DATA_DIRECTORY,
		volume, false, buffer, sizeof(buffer));
	strncat(buffer, "/openjazz/", sizeof(buffer));
	firstPath = new Path(firstPath, createString(buffer));

	result = find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY,
		volume, false, buffer, sizeof(buffer));
	strncat(buffer, "/openjazz/", sizeof(buffer));
	firstPath = new Path(firstPath, createString(buffer));
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

#ifdef _WIN32
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

	// Use the path of the program, but not on Wii as this does crash in
	// dolphin emulator. Also is not needed, because CWD is used there

#ifndef WII
	count = strlen(argv[0]) - 1;

	// Search for directory separator
#ifdef _WIN32
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
#endif


	// Use the user's home directory, if available

#ifdef HOMEDIR
	#ifdef _WIN32
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

	// Create the network address
	netAddress = createString(NET_ADDRESS);


	// Load settings from config file
	setup.load(&screenW, &screenH, &fullscreen, &scaleFactor);


	// Get command-line override
	for (count = 1; count < argc; count++) {

		// If there's a hyphen, it should be an option
		if (argv[count][0] == '-') {

#ifndef FULLSCREEN_ONLY
			if (argv[count][1] == 'f') fullscreen = true;
#endif
			if (argv[count][1] == 'm') {
				setMusicVolume(0);
				setSoundVolume(0);
			}

		}

	}


	// Create the game's window

	canvas = NULL;

	if (!video.init(screenW, screenH, fullscreen)) {

		delete firstPath;

		throw E_VIDEO;

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

		file = new File("PANEL.000", false);

	} catch (int e) {

		closeAudio();

		delete firstPath;

		log("Unable to find game data files. When launching OpenJazz, pass the location");
		log("of the original game data, eg:");
		log("  OpenJazz ~/jazz1");

#ifdef __HAIKU__
		char alertBuffer[100+B_PATH_NAME_LENGTH+B_FILE_NAME_LENGTH];
		strcpy(alertBuffer, "Unable to find game data files!\n"
			"Put the data into the folder:\n");
		strncat(alertBuffer, buffer, sizeof(alertBuffer));
		BAlert* alert = new BAlert("OpenJazz", alertBuffer, "Exit", NULL, NULL,
			B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->Go();
#endif

		throw e;

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
		font2 = new Font("FONT2.0FN");
		fontbig = new Font("FONTBIG.0FN");
		fontiny = new Font("FONTINY.0FN");
		fontmn1 = new Font("FONTMN1.0FN");
		fontmn2 = new Font("FONTMN2.0FN");

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

		throw e;

	}

	delete[] pixels;


	// Establish arbitrary timing
	globalTicks = SDL_GetTicks() - 20;


	// Fill trigonometric function look-up tables
	for (count = 0; count < 1024; count++)
		sinLut[count] = fixed(sinf(2 * PI * float(count) / 1024.0f) * 1024.0f);


	// Initiate networking
	net = new Network();


	level = NULL;
	jj2Level = NULL;

}


/**
 * De-initialises OpenJazz.
 *
 * Frees data, writes configuration, and shuts down SDL.
 */
void shutDown () {

	delete net;

	delete panelBigFont;
	delete panelSmallFont;
	delete font2;
	delete fontbig;
	delete fontiny;
	delete fontmn1;
	delete fontmn2;

#ifdef SCALE
	if (video.getScaleFactor() > 1) SDL_FreeSurface(canvas);
#endif

	closeAudio();


	// Save settings to config file
	setup.save();


	delete firstPath;

}


/**
 * Run the cutscenes and the main menu.
 *
 * @return Error code
 */
int play () {

	MainMenu *mainMenu = NULL;
	JJ1Scene *scene = NULL;

	// Load and play the startup cutscene

	try {

		scene = new JJ1Scene("STARTUP.0SC");

	} catch (int e) {

		return e;

	}

	if (scene->play() == E_QUIT) {

		delete scene;

		return E_NONE;

	}

	delete scene;


	// Load and run the menu

	try {

		mainMenu = new MainMenu();

	} catch (int e) {

		return e;

	}

	if (mainMenu->main() == E_QUIT) {

		delete mainMenu;

		return E_NONE;

	}

	delete mainMenu;


	// Load and play the ending cutscene

	try {

		scene = new JJ1Scene("END.0SC");

	} catch (int e) {

		return e;

	}

	scene->play();

	delete scene;


	return E_NONE;

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

	if (globalTicks - prevTicks < 4) {

		// Limit framerate
		SDL_Delay(4 + prevTicks - globalTicks);
		globalTicks = SDL_GetTicks();

	}

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

#ifdef PSP
	PSP_MODULE_INFO("OpenJazz", PSP_MODULE_USER, 0, 1);
	PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
	PSP_HEAP_SIZE_KB(-2048);
#endif

/**
 * Main.
 *
 * Initialises SDL and launches game.
 */
int main(int argc, char *argv[]) {

	int ret;

	// Early platform init

#ifdef PSP
	pspDebugScreenInit();
	atexit(sceKernelExitGame);
	sceIoChdir("ms0:/PSP/GAME/OpenJazz");
#elif defined(WII)
	fatInitDefault();
	Wii_SetConsole();
#endif

	// Initialise SDL

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {

		logError("Could not start SDL", SDL_GetError());

		return -1;

	}


	// Load configuration and establish a window

	try {

		startUp(argc, argv);

	} catch (int e) {

		SDL_Quit();

		return -1;

	}


	// Play the opening cutscene, run the main menu, etc.

	ret = play();


	// Save configuration and shut down

	shutDown();

	SDL_Quit();

	return ret;

}


