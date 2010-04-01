
/*
 *
 * bonus.cpp
 *
 * 23rd August 2005: Created bonus.c
 * 3rd February 2009: Renamed bonus.c to bonus.cpp
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
 * Deals with the loading, running and freeing of bonus levels.
 *
 */


#include "bonus.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "menu/menu.h"
#include "player/player.h"

#include <math.h>


int Bonus::loadTiles (char *fileName) {

	File *file;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}

	file->skipRLE();

	// Load palette
	file->loadPalette(palette);

	// Load tile graphics
	tileSet = file->loadSurface(32, 32 * 60);

	delete file;

	return E_NONE;

}


Bonus::Bonus (char * fileName, unsigned char diff) {

	File *file;
	unsigned char *buffer;
	char *string, *fileString;
	int x, y;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}


	// Load tileset

	file->seek(90, true);
	string = file->loadString();
	fileString = createFileName(string, 0);
	x = loadTiles(fileString);
	delete[] string;
	delete[] fileString;

	if (x != E_NONE) throw x;


	// Load music

	file->seek(121, true);
	fileString = file->loadString();
	playMusic(fileString);
	delete[] fileString;


	// Load tiles

	file->seek(2694, true);
	buffer = file->loadRLE(BLW * BLH);

	for (y = 0; y < BLH; y++) {

		for (x = 0; x < BLW; x++) {

			tiles[y][x] = buffer[x + (y * BLW)];

			if (tiles[y][x] > 59) tiles[y][x] = 59;

		}

	}

	delete[] buffer;


	file->skipRLE(); // Mysterious block


	// Load events

	buffer = file->loadRLE(BLW * BLH);

	for (y = 0; y < BLW; y++) {

		for (x = 0; x < BLH; x++) {

			events[y][x] = buffer[x + (y * BLW)];

		}

	}

	delete[] buffer;


	delete file;


	// Palette animations

	// Free any existing palette effects
	if (firstPE) delete firstPE;

	// Spinny whirly thing
	firstPE = new RotatePaletteEffect(112, 16, F32, NULL);

	// Track sides
	firstPE = new RotatePaletteEffect(192, 16, F32, firstPE);

	// Bouncy things
	firstPE = new RotatePaletteEffect(240, 16, -F32, firstPE);


	// Apply the palette to surfaces that already exist, e.g. fonts
	usePalette(palette);

	// Adjust fontmn1 to use bonuslevel palette
	fontmn1->mapPalette(224, 8, 0, 16);


	// Set the tick at which the level will end
	endTime = (5 - diff) * 30 * 1000;


	return;

}


Bonus::~Bonus () {

	// Free the palette effects
	if (firstPE) {

		delete firstPE;
		firstPE = NULL;

	}

	SDL_FreeSurface(tileSet);

	fontmn1->restorePalette();

	return;

}


int Bonus::play () {

	const char *options[3] = {"continue game", "setup options", "quit game"};
	PaletteEffect *levelPE;
	bool pmenu, pmessage;
	int stats, option;
	SDL_Rect src, dst;
	int x, y;
	int msps;


	tickOffset = globalTicks;
	ticks = 16;
	prevStepTicks = 0;

	pmessage = pmenu = false;
	option = 0;
	stats = S_NONE;

	// Arbitrary position
	localPlayer->setPosition(TTOF(32) + F16, TTOF(7) + F16);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) {

			pmenu = !pmenu;
			option = 0;

		}

		if (controls.release(C_PAUSE)) pmessage = !pmessage;

		if (controls.release(C_STATS)) {

			if (!gameMode) stats ^= S_SCREEN;
			else stats = (stats + 1) & 3;

		}

		if (pmenu) {

			// Deal with menu controls

			if (controls.release(C_UP)) option = (option + 2) % 3;

			if (controls.release(C_DOWN)) option = (option + 1) % 3;

			if (controls.release(C_ENTER)) {

				switch (option) {

					case 0: // Continue

						pmenu = false;

						break;

					case 1: // Setup

						if (!gameMode) {

							// Don't want palette effects in setup menu
							levelPE = firstPE;
							firstPE = NULL;

							if (menu->setup() == E_QUIT) return E_QUIT;

							// Restore level palette
							usePalette(palette);

							// Restore palette effects
							firstPE = levelPE;

						}

						break;

					case 2: // Quit game

						return E_NONE;

				}

			}

		}

		if (!gameMode) paused = pmessage || pmenu;

		timeCalcs();

		// Milliseconds per step
		msps = ticks - prevStepTicks;
		prevStepTicks = ticks;


		if (!paused) {

			// Apply controls to local player
			for (x = 0; x < PCONTROLS; x++)
				localPlayer->setControl(x, controls.getState(x));

			// Process players
			for (x = 0; x < nPlayers; x++) players[x].bonusStep(ticks, msps);

		}

		src.x = 0;
		src.w = 32;
		src.h = 32;

		int vX = FTOI(localPlayer->getX()) - (canvasW >> 1);
		int vY = FTOI(localPlayer->getY()) - (canvasH >> 1);

		for (y = 0; y <= ITOT(canvasH - 1) + 1; y++) {

			for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

				src.y = TTOI(tiles[(y + ITOT(vY) + BLH) % BLH][(x + ITOT(vX) + BLW) % BLW]);
				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);

				SDL_BlitSurface(tileSet, &src, canvas, &dst);

				dst.x = 12 + TTOI(x) - (vX & 31);
				dst.y = 12 + TTOI(y) - (vY & 31);

				switch (events[(y + ITOT(vY) + BLH) % BLH][(x + ITOT(vX) + BLW) % BLW]) {

					case 0: // No event

						break;

					case 1: // Extra time

						drawRect(dst.x, dst.y, 8, 8, 60);

						break;

					case 2: // Gem

						drawRect(dst.x, dst.y, 8, 8, 67);

						break;

					case 3: // Hand

						drawRect(dst.x, dst.y, 8, 8, 15);

						break;

					case 4: // Exit

						drawRect(dst.x, dst.y, 8, 8, 45);

						break;

					default:

						drawRect(dst.x, dst.y, 8, 8, 0);

						break;

				}

			}

		}

		// Draw the "player"
		drawRect(
			(canvasW >> 1) + fixed(sin(localPlayer->getDirection() * 6.283185 / 1024.0) * 3) - 4,
			(canvasH >> 1) - fixed(cos(localPlayer->getDirection() * 6.283185 / 1024.0) * 3) - 4, 8, 8, 0);
		drawRect((canvasW >> 1) - 4, (canvasH >> 1) - 4, 8, 8, 22);


		// Show time remaining
		if (endTime > ticks) x = endTime - ticks;
		else x = 0;
		y = x / (60 * 1000);
		fontmn1->showNumber(y, 144, 8);
		x -= (y * 60 * 1000);
		y = x / 1000;
		fontmn1->showNumber(y, 192, 8);


		// If paused, draw "PAUSE"
		if (paused && !pmenu)
			fontmn1->showString("pause", (canvasW >> 1) - 44, 32);

		// Draw statistics
		drawStats(stats);

		// Draw the menu
		if (pmenu) {

			// Draw the menu

			drawRect((canvasW >> 2) - 8, (canvasH >> 1) - 46, 144, 60, 0);

			for (x = 0; x < 3; x++) {

				if (x == option) fontmn2->mapPalette(240, 8, 31, 16);
				else fontmn2->mapPalette(240, 8, 0, 16);

				fontmn2->showString(options[x], canvasW >> 2, (canvasH >> 1) + (x << 4) - 38);

			}

			fontmn2->restorePalette();

		}

	}

	return E_NONE;

}


