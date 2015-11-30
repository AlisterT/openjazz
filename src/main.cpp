
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
 * 21st July 2013: Created setup.cpp from parts of main.cpp and setupmenu.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
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

#include <string.h>

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	#include "platforms/wiz.h"
#endif


class Main {

	public:
		Main  (int argc, char *argv[]);
		~Main ();

		int play ();

};


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
 */
Main::Main (int argc, char *argv[]) {

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


	// Load settings from config file
	setup.load(&screenW, &screenH, &fullscreen, &scaleFactor);


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

		file = new File(F_PANEL, false);

	} catch (int e) {

		closeAudio();

		delete firstPath;

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
Main::~Main () {

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
int Main::play () {

	MainMenu *mainMenu = NULL;
	JJ1Scene *scene = NULL;

	// Load and play the startup cutscene

	try {

		scene = new JJ1Scene(F_STARTUP_0SC);

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

		scene = new JJ1Scene(F_END_0SC);

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

	if (globalTicks - prevTicks < T_ACTIVE_FRAME) {

		// Limit framerate
		SDL_Delay(T_ACTIVE_FRAME + prevTicks - globalTicks);
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


/**
 * Main.
 *
 * Initialises SDL and launches game.
 */
int main(int argc, char *argv[]) {

	Main* mainObj;
	int ret;

	// Initialise SDL

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {

		logError("Could not start SDL", SDL_GetError());

		return -1;

	}


	// Load configuration and establish a window

	try {

		mainObj = new Main(argc, argv);

	} catch (int e) {

		SDL_Quit();

		return -1;

	}


	// Play the opening cutscene, run the main menu, etc.

	ret = mainObj->play();


	// Save configuration and shut down

	delete mainObj;

	SDL_Quit();

	return ret;

}


