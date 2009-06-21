
/*
 *
 * main.cpp
 *
 * Created as main.c on the 23rd of August 2005
 * Renamed main.cpp on the 3rd of February 2009
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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


#define Extern
#include "font.h"
#include "game.h"
#include "level.h"
#include "menu.h"
#include "network.h"
#include "palette.h"
#include "sound.h"
#include <string.h>


int oldTicks;
bool fakePalette;


int loadMain () {

	File *file;
	unsigned char *pixels, *sorted;
	int count, x, y;


	// Initialise video settings
	screenW = 320;
	screenH = 200;
#ifndef FULLSCREEN_ONLY
	fullscreen = false;
#endif
	// Assume that in windowed mode the palette is being emulated
	// This is extremely likely
	// To do: Find a better way!
	fakePalette = true;
	firstPE = NULL;


	// Initialise controls
	keys[C_UP].key     = SDLK_UP;
	keys[C_DOWN].key   = SDLK_DOWN;
	keys[C_LEFT].key   = SDLK_LEFT;
	keys[C_RIGHT].key  = SDLK_RIGHT;
#ifdef WIN32
	keys[C_JUMP].key   = SDLK_RALT;
	keys[C_FIRE].key   = SDLK_SPACE;
#else
	keys[C_JUMP].key   = SDLK_SPACE;
	keys[C_FIRE].key   = SDLK_LALT;
#endif
	keys[C_CHANGE].key = SDLK_RCTRL;
	keys[C_ENTER].key  = SDLK_RETURN;
	keys[C_ESCAPE].key = SDLK_ESCAPE;
	keys[C_STATS].key  = SDLK_F9;
	keys[C_PAUSE].key  = SDLK_p;

	buttons[C_UP].button = -1;
	buttons[C_DOWN].button = -1;
	buttons[C_LEFT].button = -1;
	buttons[C_RIGHT].button = -1;
	buttons[C_JUMP].button = 1;
	buttons[C_FIRE].button = 0;
	buttons[C_CHANGE].button = 3;
	buttons[C_ENTER].button = 0;
	buttons[C_ESCAPE].button = -1;
	buttons[C_STATS].button = -1;
	buttons[C_PAUSE].button = -1;

	axes[C_UP].axis = 1;
	axes[C_UP].direction = 0;
	axes[C_DOWN].axis = 1;
	axes[C_DOWN].direction = 1;
	axes[C_LEFT].axis = 0;
	axes[C_LEFT].direction = 0;
	axes[C_RIGHT].axis = 0;
	axes[C_RIGHT].direction = 1;
	axes[C_JUMP].axis = -1;
	axes[C_FIRE].axis = -1;
	axes[C_CHANGE].axis = -1;
	axes[C_ENTER].axis = -1;
	axes[C_ESCAPE].axis = -1;
	axes[C_STATS].axis = -1;
	axes[C_PAUSE].axis = -1;

	for (count = 0; count < CONTROLS; count++) {

		controls[count].time = 0;
		controls[count].state = false;

	}


	// Create the player's name
	characterName = new char[STRING_LENGTH + 1];
	strcpy(characterName, CHAR_NAME);

	// Assign the player's colour
	characterCols[0] = CHAR_FUR;
	characterCols[1] = CHAR_BAND;
	characterCols[2] = CHAR_GUN;
	characterCols[3] = CHAR_WBAND;


	// Create the network address
	netAddress = cloneString(NET_ADDRESS);


	// Open config file
	try {

		file = new File(CONFIG_FILE, false);

	} catch (int e) {

		file = NULL;

	}

	// Check that the config file was opened, and has the correct version
	if (file && (file->loadChar() == 1)) {

		// Read video settings
		screenW = file->loadShort();
		screenH = file->loadShort();
#ifdef FULLSCREEN_ONLY
		file->loadChar();
#else
		fullscreen = file->loadChar();
#endif

		// Read controls
		for (count = 0; count < CONTROLS - 4; count++)
			keys[count].key = (SDLKey)(file->loadInt());

		for (count = 0; count < CONTROLS; count++)
			buttons[count].button = file->loadInt();

		for (count = 0; count < CONTROLS; count++) {

			axes[count].axis = file->loadInt();
			axes[count].direction = file->loadInt();

		}

		// Read the player's name
		for (count = 0; count < STRING_LENGTH; count++)
			characterName[count] = file->loadChar();

		characterName[STRING_LENGTH] = 0;

		// Read the player's colours
		characterCols[0] = file->loadChar();
		characterCols[1] = file->loadChar();
		characterCols[2] = file->loadChar();
		characterCols[3] = file->loadChar();

		delete file;

	} else {

		printf("Valid configuration file not found.\n");

	}

	// Create the game's window

#ifdef FULLSCREEN_ONLY
	screen = SDL_SetVideoMode(screenW, screenH, 8,
		SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);
#else
	screen = SDL_SetVideoMode(screenW, screenH, 8,
		SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE);
#endif

	if (!screen) {

		fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());

		delete characterName;

		return E_VIDEO;

	}

	SDL_WM_SetCaption("OpenJazz", NULL);


	if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);


	// Generate the logical palette
	for (count = 0; count < 256; count++)
		logicalPalette[count].r = logicalPalette[count].g =
 			logicalPalette[count].b = count;

	restorePalette(screen);


	// Set up audio
	openAudio();



	// Load the panel

	try {

		file = new File(F_PANEL, false);

	} catch (int e) {

		closeAudio();

		delete characterName;

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

		delete file;

		return e;

	}


	delete file;


	// Establish arbitrary timing
	mspf = 20;
	oldTicks = SDL_GetTicks() - 20;


	// Initiate networking
	net = new Network();


	return E_NONE;

}


void freeMain () {

	File *file;
	int count;

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
		file->storeChar(1);

		// Write video settings
		file->storeShort(screenW);
		file->storeShort(screenH);
#ifdef FULLSCREEN_ONLY
		file->storeChar(1);
#else
		file->storeChar(fullscreen? ~0: 0);
#endif

		// Write controls
		for (count = 0; count < CONTROLS - 4; count++)
			file->storeInt(keys[count].key);

		for (count = 0; count < CONTROLS; count++)
			file->storeInt(buttons[count].button);

		for (count = 0; count < CONTROLS; count++) {

			file->storeInt(axes[count].axis);
			file->storeInt(axes[count].direction);

		}

		// Write the player's name
		for (count = 0; count < STRING_LENGTH; count++)
			file->storeChar(characterName[count]);

		// Write the player's colour
		file->storeChar(characterCols[0]);
		file->storeChar(characterCols[1]);
		file->storeChar(characterCols[2]);
		file->storeChar(characterCols[3]);

		delete file;

	} else {

		fprintf(stderr, "Could not write configuration file.\n");

	}

	delete[] characterName;


	return;

}


void releaseControl (int control) {

	controls[control].time = oldTicks + T_KEY;
	controls[control].state = false;

	return;

}


int loop (int type) {

	SDL_Color shownPalette[256];
	SDL_Event event;
	int count;


	// Show everything that has been drawn so far
	SDL_Flip(screen);

	// Calculate frame rate and key timing
	count = SDL_GetTicks();
	mspf = count - oldTicks;
	if (mspf > 100) mspf = 100;
	oldTicks = count;


	// Process system events
	while (SDL_PollEvent(&event)) {

		switch (event.type) {

			case SDL_KEYDOWN:

#ifndef FULLSCREEN_ONLY
				// If Alt + Enter has been pressed, go to full screen
				if ((event.key.keysym.sym == SDLK_RETURN) &&
					(event.key.keysym.mod & KMOD_ALT)) {

					fullscreen = !fullscreen;

					if (fullscreen) {

						SDL_ShowCursor(SDL_DISABLE);
						screen = SDL_SetVideoMode(screenW, screenH, 8,
							SDL_FULLSCREEN | SDL_DOUBLEBUF | SDL_HWSURFACE |
							SDL_HWPALETTE);
						SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0,
							256);
						SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0,
							256);

						/* A real 8-bit display is quite likely if the user has
						the right video card, the right video drivers, the right
						version of DirectX/whatever, and the right version of
						SDL. In other words, it's not likely enough.
						If a real palette is assumed when
						a) there really is a real palette, there will be an
							extremely small speed gain.
						b) the palette is emulated, there will be a HUGE speed
							loss.
						Therefore, assume the palette is emulated.
						To do: Find a better way! */
						fakePalette = true;

					} else {

						screen = SDL_SetVideoMode(screenW, screenH, 8,
							SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE |
							SDL_HWPALETTE);
						SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0,
							256);
						SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0,
							256);
						SDL_ShowCursor(SDL_ENABLE);

						/* Assume that in windowed mode the palette is being
						emulated.
						This is extremely likely.
						To do: Find a better way! */
						fakePalette = true;

					}

				}

				// The absence of a break statement is intentional
#endif

			case SDL_KEYUP:

				for (count = 0; count < CONTROLS; count++)
					if (event.key.keysym.sym == keys[count].key)
						keys[count].state = event.key.state;

				break;

			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:

				for (count = 0; count < CONTROLS; count++)
					if (event.jbutton.button == buttons[count].button)
						buttons[count].state = event.jbutton.state;

				break;

			case SDL_JOYAXISMOTION:

				for (count = 0; count < CONTROLS; count++)
					if (event.jaxis.axis == axes[count].axis) {

						if (!axes[count].direction &&
							(event.jaxis.value < -16384))
							axes[count].state = SDL_PRESSED;

						else if (axes[count].direction &&
							(event.jaxis.value > 16384))
							axes[count].state = SDL_PRESSED;

						else axes[count].state = SDL_RELEASED;

					}

				break;

#ifndef FULLSCREEN_ONLY
			case SDL_VIDEORESIZE:

				screenW = event.resize.w;
				screenH = event.resize.h;
				screen = SDL_SetVideoMode(screenW, screenH, 8,
					SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_HWSURFACE |
					SDL_HWPALETTE);

				// The absence of a break statement is intentional

			case SDL_VIDEOEXPOSE:

				SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
				SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

				break;
#endif

			case SDL_QUIT:

				return E_QUIT;

		}

		if ((type == KEY_LOOP) && (event.type == SDL_KEYDOWN)) {

			return event.key.keysym.sym;

		}

		if ((type == JOYSTICK_LOOP) && (event.type == SDL_JOYBUTTONDOWN)) {

			return JOYSTICKB | event.jbutton.button;

		}

		if ((type == JOYSTICK_LOOP) && (event.type == SDL_JOYAXISMOTION)) {

			if (event.jaxis.value < -16384)
				return JOYSTICKANEG | event.jaxis.axis;

			if (event.jaxis.value > 16384)
				return JOYSTICKAPOS | event.jaxis.axis;

		}

	}

	// Apply controls to universal control tracking
	for (count = 0; count < CONTROLS; count++) {

		if ((keys[count].state == SDL_PRESSED) ||
			(buttons[count].state == SDL_PRESSED) ||
			(axes[count].state == SDL_PRESSED)) {

			if (controls[count].time < oldTicks) controls[count].state = true;

		} else {

			controls[count].time = 0;
			controls[count].state = false;

		}

	}


	// Apply palette effects

	if (firstPE) {

		/* If the palette is being emulated, compile all palette changes and
		apply them all at once.
		If the palette is being used directly, apply all palette effects
		directly. */

		if (fakePalette) {

			memcpy(shownPalette, currentPalette, sizeof(SDL_Color) * 256);

			firstPE->apply(shownPalette, false);

			SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

		} else {

			firstPE->apply(shownPalette, true);

		}

	}

	return E_NONE;

}


int main(int argc, char *argv[]) {

	// Find path

	if (argc < 2) {

		// No path was given, use the path of the program

		int count;

		count = strlen(argv[0]) - 1;

		// Search for directory separator
#ifdef WIN32
		while ((argv[0][count] != '\\') && (count >= 0)) count--;
#else
		while ((argv[0][count] != '/') && (count >= 0)) count--;
#endif

		path = new char[count + 2];

		// If a directory was found, copy it to the path
		if (count >= 0) {

			memcpy(path, argv[0], count + 1);
			path[count + 1] = 0;

		} else path[0] = 0;

	} else {

		// Copy the provided path, appending a directory separator as necessary

#ifdef WIN32
		if (argv[1][strlen(argv[1]) - 1] != '\\') {
#else
		if (argv[1][strlen(argv[1]) - 1] != '/') {
#endif

			path = new char[strlen(argv[1]) + 2];
			strcpy(path, argv[1]);

#ifdef WIN32
			strcat(path, "\\");
#else
			strcat(path, "/");
#endif

		} else {

			path = cloneString(argv[1]);

		}

	}


	// Initialise SDL

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK |
		SDL_INIT_TIMER) < 0) {

		fprintf(stderr, "Could not start SDL: %s\n", SDL_GetError());

		return -1;

	}


	// Load universal game data and establish a window

	if (loadMain() != E_NONE) {

		SDL_Quit();
		delete[] path;

		return -1;

	}


	// Show the startup cutscene
//	if (runScene("startup.0sc") != E_QUIT) {

		// Load the menu
		try {

			menu = new Menu();

		} catch (int e) {

			freeMain();
			SDL_Quit();
			delete[] path;

			return e;

		}

		// Run the menu
		if (menu->run() == E_NONE) {

			// Show the ending cutscene
//			runScene("end.0sc");

		}

		delete menu;

//	}

	freeMain();
	SDL_Quit();
	delete[] path;


	return 0;

}


