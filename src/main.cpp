
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
 * Copyright (c) 2005-2017 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Contains the main function.
 *
 */


// consume all external variables
#define EXTERN

#include "game/game.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/network.h"
#include "io/sound.h"
#ifdef ENABLE_JJ2
#include "jj2/level/jj2level.h"
#endif
#include "jj1/level/jj1level.h"
#include "menu/menu.h"
#include "player/player.h"
#include "jj1/scene/jj1scene.h"
#include "loop.h"
#include "setup.h"
#include "util.h"
#include "io/log.h"
#include "platforms/platforms.h"
#include "version.h"

#include <cstring>
#include <argparse.h>

#ifndef __SYMBIAN32__
	#include <math.h>
#endif

#define PI 3.141592f

// Fallback links, should be provided by the build system
#ifndef OJ_BUGREPORT
	#define OJ_BUGREPORT "https://github.com/AlisterT/openjazz/issues"
#endif
#ifndef OJ_URL
	#define OJ_URL "http://alister.eu/jazz/oj/"
#endif

static struct CliOptions {
	bool muteAudio;
	int fullScreen;
	int scaleFactor;
	int level;
	int world;
	char *verboseLevel;
	int quiet;
} cli = {
	false, -1, -1, -1, -1, NULL, 0
};

#ifndef FULLSCREEN_ONLY
int display_mode_cb(struct argparse *, const struct argparse_option *option) {
	cli.fullScreen = (option->short_name == 'f') ? 1 : 0;
	return 0;
}
#endif

int version_cb(struct argparse *, const struct argparse_option */*option*/) {
	printf("OpenJazz %s, built on %s.\n", oj_version, oj_date);
	exit(EXIT_SUCCESS);
}

int checkOptions (int argc, char *argv[]) {

	struct argparse argparse;

	// generic usage message
	const char *usage[] = {
		"OpenJazz [options] [[--] <game directory 1> <game directory 2>]",
	    NULL,
	};

	// command line options
	struct argparse_option opt[] = {
		OPT_HELP(),
		OPT_GROUP("Engine options"),
		OPT_BOOLEAN('m', "mute", &cli.muteAudio, "Mute audio output", NULL, 0, 0),
#ifndef FULLSCREEN_ONLY
		OPT_BOOLEAN('f', "fullscreen", NULL, "Display in Fullscreen mode",
			display_mode_cb, 0, OPT_NONEG),
		OPT_BOOLEAN('\0', "window", NULL, "Display in Window mode",
			display_mode_cb, 0, OPT_NONEG),
#endif
		OPT_INTEGER('s', "scale", &cli.scaleFactor, "Scale graphics <int> times", NULL, 0, 0),
		OPT_GROUP("Developer options"),
		OPT_INTEGER('w', "world", &cli.world, "Load specific World", NULL, 0, 0),
		OPT_INTEGER('l', "level", &cli.level, "Load specific Level", NULL, 0, 0),
		OPT_BOOLEAN('q', "quiet", &cli.quiet, "Disable console logging (Enable with --no-quiet)", NULL, 0, 0),
		OPT_STRING('\0', "verbose", &cli.verboseLevel,
			"Verbosity level: max, trace, debug, info, warn, error, fatal", NULL, 0, 0),
		OPT_BOOLEAN('v', "version", NULL, "Show version information", version_cb, 0, OPT_NONEG),
		OPT_END(),
	};

	argparse_init(&argparse, opt, usage, 0);
	argparse_describe(&argparse,
		"\nOpenJazz - Jack Jazzrabbit 1 game engine reimplementation",
		"\nBug reports: " OJ_BUGREPORT " - Homepage: " OJ_URL);
	argc = argparse_parse(&argparse, argc, argv);

	// apply logger options
	if (cli.quiet) logger.setQuiet(cli.quiet);
	int verbosity = logger.getLevel();
	if (cli.verboseLevel) {
		if (!strcmp(cli.verboseLevel, "max"))        verbosity = LL_MAX;
		else if (!strcmp(cli.verboseLevel, "trace")) verbosity = LL_TRACE;
		else if (!strcmp(cli.verboseLevel, "debug")) verbosity = LL_DEBUG;
		else if (!strcmp(cli.verboseLevel, "info"))  verbosity = LL_INFO;
		else if (!strcmp(cli.verboseLevel, "warn"))  verbosity = LL_WARN;
		else if (!strcmp(cli.verboseLevel, "error")) verbosity = LL_ERROR;
		else if (!strcmp(cli.verboseLevel, "fatal")) verbosity = LL_FATAL;
		else {

			fprintf(stderr, "error: option `--verbose` has invalid level\n");
			exit(EXIT_FAILURE);

		}
	}
	logger.setLevel(verbosity);

	return argc;
}


/**
 * Initialises OpenJazz.
 *
 * Establishes the paths from which to read files, loads configuration, sets up
 * the game window and loads required data.
 *
 * @param argv0 program path
 * @param pathCount Number of path arguments
 * @param paths Array of path argument strings
 */
void startUp (const char *argv0, int pathCount, char *paths[]) {

	File* file;
	unsigned char* pixels = NULL;
	SetupOptions config;

	// Determine paths

	PLATFORM_AddGamePaths();

	// Use any provided paths
	for (int i = 0; i < pathCount; i++)
		gamePaths.add(createString(paths[i]), PATH_TYPE_GAME);


	// Use the path of the program, but check before, since it is not always available
	// At least crashes in Dolphin emulator (Wii) and 3DS (.cia build)
	if (argv0) {

		int i = strlen(argv0) - 1;

		// Search for directory separator
		while ((argv0[i] != OJ_DIR_SEP) && (i > 0)) i--;

		// If a directory was found, copy it to the path
		if (i > 0) {

			char *dir = createString(argv0);
			dir[i+1] = '\0';
			gamePaths.add(dir, PATH_TYPE_SYSTEM|PATH_TYPE_GAME);

		}

	}

	// Use the current working directory
	gamePaths.add(createString(""), PATH_TYPE_GAME|PATH_TYPE_CONFIG|PATH_TYPE_TEMP);

	// Use hard-coded data paths, if available
#ifdef DATAPATH
	gamePaths.add(createString(DATAPATH), PATH_TYPE_SYSTEM|PATH_TYPE_GAME);
#endif

	// Default settings

	// Sound settings
#if defined(WIZ) || defined(GP2X)
	volume = 40;
#endif

	// Create the network address
	netAddress = createString(NET_ADDRESS);


	// Load settings from config file
	config = setup.load();
	if (!config.valid) {

		// Invalid config - apply defaults
		config.videoWidth = DEFAULT_SCREEN_WIDTH;
		config.videoHeight = DEFAULT_SCREEN_HEIGHT;
		config.videoScale = 1;

	}

	// Apply command-line override
	if (cli.fullScreen > -1) config.fullScreen = cli.fullScreen;
	if (cli.scaleFactor > 0) config.videoScale = cli.scaleFactor;
	if (cli.muteAudio) {

		setMusicVolume(0);
		setSoundVolume(0);

	}


	// Create the game's window
	canvas = NULL;
	if (!video.init(config)) {

		throw E_VIDEO;

	}


	if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);


	// Set up audio
	openAudio();


	// Load fonts

	// Open the panel, which contains two fonts

	try {

		file = new File("PANEL.000", PATH_TYPE_GAME);

	} catch (int e) {

		closeAudio();

		LOG_FATAL("Unable to find game data files. When launching OpenJazz, \n"
			"               pass the location of the original game data, eg:\n"
			"                 OpenJazz ~/jazz1");

		PLATFORM_ErrorNoDatafiles();

		throw;

	}

	pixels = file->loadRLE(46272);

	delete file;

	panelBigFont = nullptr;
	panelSmallFont = nullptr;
	font2 = nullptr;
	fontbig = nullptr;
	fontiny = nullptr;
	fontmn1 = nullptr;

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

		throw;

	}
	#if DEBUG_FONTS
	panelBigFont->saveAtlasAsBMP("panelbigfont.bmp");
	panelSmallFont->saveAtlasAsBMP("panelsmallfont.bmp");
	font2->saveAtlasAsBMP("font2.bmp");
	fontbig->saveAtlasAsBMP("fontbig.bmp");
	fontiny->saveAtlasAsBMP("fontiny.bmp");
	fontmn1->saveAtlasAsBMP("fontmn1.bmp");
	#endif

	delete[] pixels;

	// Establish arbitrary timing
	globalTicks = SDL_GetTicks() - 20;


	// Fill trigonometric function look-up tables
	for (int i = 0; i < 1024; i++)
		sinLut[i] = fixed(sinf(2 * PI * float(i) / 1024.0f) * 1024.0f);


	// Initiate networking
	net = new Network();


	level = NULL;

#ifdef ENABLE_JJ2
	jj2Level = NULL;
#endif

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

	closeAudio();

	video.deinit();

	// Save settings to config file
	setup.save();

}


/**
 * Run the cutscenes and the main menu.
 *
 * @return Error code
 */
int play () {

	MainMenu *mainMenu = NULL;
	JJ1Scene *scene = NULL;

	// Start the opening music

	playMusic("MENUSNG.PSM");

	// Try loading the user-specified level

	if (cli.level > -1 && cli.world > -1) {

		try {

			mainMenu = new MainMenu();

		} catch (int e) {

			return e;

		}

		if (mainMenu->skip(cli.level, cli.world) == E_QUIT) {

			delete mainMenu;

		}

		return E_NONE;

	}


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
 * @param effectsStopped Whether the effects should be applied without advancing
 *
 * @return Error code
 */
int loop (LoopType type, PaletteEffect* paletteEffects, bool effectsStopped) {

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
	video.flip(globalTicks - prevTicks, paletteEffects, effectsStopped);


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

#ifdef PSP
	if (PSP_WantsExit()) return E_QUIT;
#elif defined(WIZ) || defined(GP2X)
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

	int ret;
	const char *argv0 = NULL;

	PLATFORM_Init();

	// Some platforms (and emulators) do not provide arguments

	if (argc) {

		// Save program path
		argv0 = argv[0];

		// Check command line options
		argc = checkOptions(argc, argv);

	}

	// Log current version
	LOG_INFO("This is OpenJazz %s, built on %s.", oj_version, oj_date);

	// Initialise SDL

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {

		LOG_FATAL("Could not start SDL: %s\n", SDL_GetError());

		return -1;

	}


	// Load configuration and establish a window

	try {

		startUp(argv0, argc, argv);

	} catch (int e) {

		SDL_Quit();

		return -1;

	}


	// Play the opening cutscene, run the main menu, etc.

	ret = play();


	// Save configuration and shut down

	LOG_DEBUG("Shutting down engine.");

	shutDown();

	PLATFORM_Exit();

	SDL_Quit();

	return ret;

}
