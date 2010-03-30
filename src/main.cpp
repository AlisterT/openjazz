
/*
 *
 * main.cpp
 *
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed main.c to main.cpp
 * 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * 13th July 2009: Created controls.cpp from parts of main.cpp
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
#include "level/level.h"
#include "menu/menu.h"
#include "player/player.h"
#include "scene/scene.h"

#include <string.h>

#if defined(WIZ) || defined(GP2X)
	#include "platforms/wiz.h"
#endif

#ifdef __SYMBIAN32__
extern char KOpenJazzPath[256];
#endif

#ifdef SCALE
	#include "io/gfx/scale2x/scalebit.h"
#endif

int loadMain (int argc, char *argv[]) {

	File *file;
	unsigned char *pixels, *sorted;
	int count, x, y;
#ifndef SCALE
	int scaleFactor;
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

	// Video settings
	screenW = 320;
	screenH = 200;
#ifndef FULLSCREEN_ONLY
	fullscreen = false;
#endif

	// Sound settings
#if defined(WIZ) || defined(GP2X)
	volume = 40;
#endif
	soundsVolume = MAX_VOLUME >> 2;


	// Create the player's name
	characterName = createEditableString(CHAR_NAME);

	// Assign the player's colour
	characterCols[0] = CHAR_FUR;
	characterCols[1] = CHAR_BAND;
	characterCols[2] = CHAR_GUN;
	characterCols[3] = CHAR_WBAND;


	// Create the network address
	netAddress = createString(NET_ADDRESS);


	// Open config file

	try {

		file = new File(CONFIG_FILE, false);

	} catch (int e) {

		file = NULL;

	}

	// Check that the config file was opened, and has the correct version
	if (file && (file->loadChar() == 2)) {

		// Read video settings
		screenW = file->loadShort();
		screenH = file->loadShort();

		scaleFactor = file->loadChar();
#ifndef FULLSCREEN_ONLY
		fullscreen = scaleFactor & 1;
#endif
		scaleFactor >>= 1;
		if (scaleFactor > 4) scaleFactor = 1;


		// Read controls
		for (count = 0; count < CONTROLS - 4; count++)
			controls.setKey(count, (SDLKey)(file->loadInt()));

		for (count = 0; count < CONTROLS; count++)
			controls.setButton(count, file->loadInt());

		for (count = 0; count < CONTROLS; count++)
			controls.setAxis(count, file->loadInt(), file->loadInt());

		// Read the player's name
		for (count = 0; count < STRING_LENGTH; count++)
			characterName[count] = file->loadChar();

		characterName[STRING_LENGTH] = 0;

		// Read the player's colours
		characterCols[0] = file->loadChar();
		characterCols[1] = file->loadChar();
		characterCols[2] = file->loadChar();
		characterCols[3] = file->loadChar();

		// Read the sound effect volume
		soundsVolume = file->loadChar();
		if (soundsVolume > MAX_VOLUME) soundsVolume = MAX_VOLUME;

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


	// Generate the logical palette
	for (count = 0; count < 256; count++)
		logicalPalette[count].r = logicalPalette[count].g =
 			logicalPalette[count].b = count;


	// Create the game's window

	currentPalette = logicalPalette;
	canvas = screen = NULL;

#ifndef FULLSCREEN_ONLY
	if (fullscreen)
#endif
		SDL_ShowCursor(SDL_DISABLE);

	createScreen();

	if (!screen) {

		logError("Could not set video mode", SDL_GetError());

		delete[] characterName;

		delete firstPath;

		return E_VIDEO;

	}

	SDL_WM_SetCaption("OpenJazz", NULL);


	if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);


	restorePalette(screen);

	firstPE = NULL;


	// Set up audio
	openAudio();



	// Load the panel

	try {

		file = new File(F_PANEL, false);

	} catch (int e) {

		closeAudio();

		delete[] characterName;

		delete firstPath;

		return e;

	}

	// Load the panel background
	panel = file->loadSurface(320, 32);


	// Load the panel's ammo graphics

	sorted = new unsigned char[64 * 27];

	file->seek(7537, true);
	pixels = file->loadRLE(64 * 27);

	for (y = 0; y < 27; y++) {

		for (x = 0; x < 64; x++)
			sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

	}

	panelAmmo[0] = createSurface(sorted, 64, 27);
	sorted = pixels; // Re-use the allocated memory

	file->seek(8264, true);
	pixels = file->loadRLE(64 * 27);

	for (y = 0; y < 27; y++) {

		for (x = 0; x < 64; x++)
			sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

	}

	panelAmmo[1] = createSurface(sorted, 64, 27);
	sorted = pixels; // Re-use the allocated memory

	file->seek(9550, true);
	pixels = file->loadRLE(64 * 27);

	for (y = 0; y < 27; y++) {

		for (x = 0; x < 64; x++)
			sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

	}

	panelAmmo[2] = createSurface(sorted, 64, 27);
	sorted = pixels; // Re-use the allocated memory

	file->seek(11060, true);
	pixels = file->loadRLE(64 * 27);

	for (y = 0; y < 27; y++) {

		for (x = 0; x < 64; x++)
			sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

	}

	panelAmmo[3] = createSurface(sorted, 64, 27);
	sorted = pixels; // Re-use the allocated memory

	file->seek(12258, true);
	pixels = file->loadRLE(64 * 27);

	for (y = 0; y < 27; y++) {

		for (x = 0; x < 64; x++)
			sorted[(y * 64) + x] = pixels[(y * 64) + (x >> 2) + ((x & 3) << 4)];

	}

	panelAmmo[4] = createSurface(sorted, 64, 27);
	delete[] pixels; // Don't re-use the allocated memory


	// Load fonts
	panelBigFont = NULL;
	panelSmallFont = NULL;
	font2 = NULL;
	fontbig = NULL;
	fontiny = NULL;
	fontmn1 = NULL;

	try {

		panelBigFont = new Font(file, true);
		panelSmallFont = new Font(file, false);
		font2 = new Font("font2.0fn");
		fontbig = new Font("fontbig.0fn");
		fontiny = new Font("fontiny.0fn");
		fontmn1 = new Font("fontmn1.0fn");
		fontmn2 = new Font("fontmn2.0fn");

	} catch (int e) {

		if (panelBigFont) delete panelBigFont;
		if (panelSmallFont) delete panelSmallFont;
		if (font2) delete font2;
		if (fontbig) delete fontbig;
		if (fontiny) delete fontiny;
		if (fontmn1) delete fontmn1;

		SDL_FreeSurface(panel);
		SDL_FreeSurface(panelAmmo[0]);
		SDL_FreeSurface(panelAmmo[1]);
		SDL_FreeSurface(panelAmmo[2]);
		SDL_FreeSurface(panelAmmo[3]);
		SDL_FreeSurface(panelAmmo[4]);

		closeAudio();

		delete[] characterName;

		delete firstPath;

		delete file;

		return e;

	}


	delete file;


	// Establish arbitrary timing
	globalTicks = SDL_GetTicks() - 20;


	// Initiate networking
	net = new Network();


	return E_NONE;

}


void freeMain () {

	File *file;
	int count;
#ifndef SCALE
	int scaleFactor = 1;
#endif

	delete net;

	delete panelBigFont;
	delete panelSmallFont;
	delete font2;
	delete fontbig;
	delete fontiny;
	delete fontmn1;
	delete fontmn2;

	SDL_FreeSurface(panel);
	SDL_FreeSurface(panelAmmo[0]);
	SDL_FreeSurface(panelAmmo[1]);
	SDL_FreeSurface(panelAmmo[2]);
	SDL_FreeSurface(panelAmmo[3]);
	SDL_FreeSurface(panelAmmo[4]);

#ifdef SCALE
	if (canvas != screen) SDL_FreeSurface(canvas);
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
		file->storeChar(2);

		// Write video settings
		file->storeShort(screenW);
		file->storeShort(screenH);
		scaleFactor <<= 1;
#ifndef FULLSCREEN_ONLY
		scaleFactor |= fullscreen? 1: 0;
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
			file->storeChar(characterName[count]);

		// Write the player's colour
		file->storeChar(characterCols[0]);
		file->storeChar(characterCols[1]);
		file->storeChar(characterCols[2]);
		file->storeChar(characterCols[3]);

		// Write the sound effect volume
		file->storeChar(soundsVolume);

		delete file;

	} else {

		logError("Could not write configuration file",
			"File could not be opened.");

	}

	delete[] characterName;

	delete firstPath;


	SDL_Quit();


	return;

}


int loop (int type) {

	SDL_Color shownPalette[256];
	SDL_Event event;
	int prevTicks, ret;

	// Show everything that has been drawn so far
#ifdef SCALE
	if (canvas != screen) {

		scale(scaleFactor,
			screen->pixels, screen->pitch,
			canvas->pixels, canvas->pitch,
			screen->format->BytesPerPixel, canvas->w, canvas->h);

	}
#endif

	SDL_Flip(screen);


	prevTicks = globalTicks;
	globalTicks = SDL_GetTicks();

	// Process system events
	while (SDL_PollEvent(&event)) {

		switch (event.type) {

			case SDL_KEYDOWN:

#ifndef FULLSCREEN_ONLY
				// If Alt + Enter has been pressed, go to full screen
				if ((event.key.keysym.sym == SDLK_RETURN) &&
					(event.key.keysym.mod & KMOD_ALT)) {

					fullscreen = !fullscreen;

					SDL_ShowCursor(fullscreen? SDL_DISABLE: SDL_ENABLE);
					createScreen();

				}
#endif
#if defined(WIZ) || defined(GP2X)
				SDL_ShowCursor(SDL_DISABLE);
#endif
				// Break statement intentionally omitted

			case SDL_KEYUP:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			case SDL_JOYAXISMOTION:

				ret = controls.update(&event, type);

				if (ret != E_NONE) return ret;

#if defined(WIZ) || defined(GP2X)
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
#endif
				break;

#ifndef FULLSCREEN_ONLY
			case SDL_VIDEORESIZE:

				screenW = event.resize.w;
				screenH = event.resize.h;

				createScreen();

				break;

			case SDL_VIDEOEXPOSE:

				SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
				SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

				break;
#endif

			case SDL_QUIT:

				return E_QUIT;

		}

	}

	controls.loop();


	// Apply palette effects

	if (firstPE) {

		/* If the palette is being emulated, compile all palette changes and
		apply them all at once.
		If the palette is being used directly, apply all palette effects
		directly. */

		if (fakePalette) {

			memcpy(shownPalette, currentPalette, sizeof(SDL_Color) * 256);

			firstPE->apply(shownPalette, false, globalTicks - prevTicks);

			SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

		} else {

			firstPE->apply(shownPalette, true, globalTicks - prevTicks);

		}

	}

#if defined(WIZ) || defined(GP2X)
	WIZ_AdjustVolume( volume_direction );
#endif

	return E_NONE;

}


int main(int argc, char *argv[]) {

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

/*	try {

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

	delete scene;*/


	// Load the menu
	try {

		menu = new Menu();

	} catch (int e) {

		freeMain();

		return e;

	}

	// Run the main menu
	if (menu->main() == E_NONE) {

		// Show the ending cutscene

		try {

			scene = new Scene(F_END_0SC);

		} catch (int e) {

			delete menu;
			freeMain();

			return e;

		}

		scene->play();

		delete scene;


	}

	delete menu;
	freeMain();


	return 0;

}


