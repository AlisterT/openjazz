
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
#include "game/game.h"
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
#include <string.h>


int Bonus::loadTiles (char *fileName) {

	File *file;
	unsigned char *pixels;
	unsigned char *sorted;
	int count;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return e;

	}

	// Load background
	pixels = file->loadRLE(832 * 20);
	sorted = new unsigned char[512 * 20];

	for (count = 0; count < 20; count++) memcpy(sorted + (count * 512), pixels + (count * 832), 512);

	background = createSurface(sorted, 512, 20);

	delete[] sorted;
	delete[] pixels;

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


	file->seek(178, false);

	// Set the tick at which the level will end
	endTime = file->loadShort() * 1000;


	// Number of gems to collect
	items = file->loadShort();


	// The players' coordinates
	x = file->loadShort();
	y = file->loadShort();

	if (game) game->setCheckpoint(x, y);


	// Set the players' initial values
	if (game) {

		for (x = 0; x < nPlayers; x++)
			game->resetPlayer(players + x, true);

    } else {

		localPlayer->reset();
		localPlayer->setPosition(TTOF(x) + F16, TTOF(y) + F16);

    }

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


	// Adjust fontsFont to use bonus level palette
	fontsFont->mapPalette(0, 32, 15, -16);


	return;

}


Bonus::~Bonus () {

	// Free the palette effects
	if (firstPE) {

		delete firstPE;
		firstPE = NULL;

	}

	SDL_FreeSurface(tileSet);

	fontsFont->restorePalette();

	return;

}


int Bonus::step () {

	fixed playerX, playerY;
	int gridX, gridY;
	int msps;
	int count;

	// Milliseconds per step
	msps = ticks - prevStepTicks;
	prevStepTicks = ticks;


	// Check if time has run out
	if (ticks > endTime) return LOST;


	// Apply controls to local player
	for (count = 0; count < PCONTROLS; count++)
		localPlayer->setControl(count, controls.getState(count));

	// Process players
	for (count = 0; count < nPlayers; count++) {

		players[count].bonusStep(ticks, msps);

		playerX = players[count].getX();
		playerY = players[count].getY();

		gridX = FTOT(playerX);
		gridY = FTOT(playerY);

		if ((playerX > TTOF(gridX) + F12) && (playerX < TTOF(gridX) + F20) &&
			(playerY > TTOF(gridY) + F12) && (playerY < TTOF(gridY) + F20)) {

			while (gridX < 0) gridX += BLW;
			while (gridY < 0) gridY += BLH;

			switch (events[gridY][gridX]) {

				case 1: // Extra time

					addTimer();
					events[gridY][gridX] = 0;

					break;

				case 2: // Gem

					players[count].addItem();
					events[gridY][gridX] = 0;

					if (players[count].getItems() >= items) {

						players[count].addLife();

						return WON;

					}

					break;

				case 3: // Hand

					players[count].setSpeed(0, 0);

					break;

				case 4: // Exit

					return LOST;

				default:

					// Do nothing

					break;

			}

		}

	}

	return E_NONE;

}


void Bonus::draw () {

	SDL_Rect src, dst;
	int x, y;

	// Draw the ground

	src.x = 0;
	src.w = 32;
	src.h = 32;

	int vX = FTOI(localPlayer->getX()) - (canvasW >> 1);
	int vY = FTOI(localPlayer->getY()) - (canvasH >> 2);

	for (y = 0; y <= ITOT((canvasH >> 1) - 1) + 1; y++) {

		for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

			src.y = TTOI(tiles[(y + ITOT(vY) + BLH) % BLH][(x + ITOT(vX) + BLW) % BLW]);
			dst.x = TTOI(x) - (vX & 31);
			dst.y = (canvasH >> 1) + TTOI(y) - (vY & 31);

			SDL_BlitSurface(tileSet, &src, canvas, &dst);

			dst.x = 12 + TTOI(x) - (vX & 31);
			dst.y = (canvasH >> 1) + 12 + TTOI(y) - (vY & 31);

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


	// Draw the background

	for (x = -(localPlayer->getDirection() & 1023); x < canvasW; x += background->w) {

		dst.x = x;
		dst.y = (canvasH >> 1) - 4;
		SDL_BlitSurface(background, NULL, canvas, &dst);

	}

	x = 171;

	for (y = (canvasH >> 1) - 5; (y >= 0) && (x > 128); y--) drawRect(0, y, canvasW, 1, x--);

	if (y > 0) drawRect(0, 0, canvasW, y + 1, 128);


	// Draw the "player"
	drawRect(
		(canvasW >> 1) + fixed(sin(localPlayer->getDirection() * 6.283185 / 1024.0) * 3) - 4,
		((canvasH * 3) >> 2) - fixed(cos(localPlayer->getDirection() * 6.283185 / 1024.0) * 3) - 4, 8, 8, 0);
	drawRect((canvasW >> 1) - 4, ((canvasH * 3) >> 2) - 4, 8, 8, 22);


	// Show gem count
	fontsFont->showString("x", 15, 0);
	fontsFont->showNumber(localPlayer->getItems(), 64, 0);
	fontsFont->showNumber(items, 117, 0);


	// Show time remaining
	if (endTime > ticks) x = (endTime - ticks) / 1000;
	else x = 0;
	fontsFont->showNumber(x / 60, 242, 0);
	fontsFont->showNumber(x % 60, 286, 0);


	return;

}


int Bonus::play () {

	const char *options[3] = {"continue game", "setup options", "quit game"};
	PaletteEffect *levelPE;
	bool pmenu, pmessage;
	int stats, option;
	unsigned int returnTime;
	int count;


	tickOffset = globalTicks;
	ticks = 16;
	prevStepTicks = 0;

	pmessage = pmenu = false;
	option = 0;
	stats = S_NONE;

	returnTime = 0;

	usePalette(palette);

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


		// Check if level has been won
		if (returnTime && (ticks > returnTime)) {

			if (localPlayer->getItems() >= items) return WON;

			return LOST;

		}


		// Process frame-by-frame activity

		if (!paused && (ticks >= prevStepTicks + 16) && (stage == LS_NORMAL)) {

			count = step();

			if (count < 0) return count;
			else if (count) {

				stage = LS_END;
				firstPE = new WhiteOutPaletteEffect(T_BONUS_END, firstPE);
				returnTime = ticks + T_BONUS_END;

			}

		}


		// Draw the graphics

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			fontsFont->showString("pause", (canvasW >> 1) - 44, 32);

		// Draw statistics
		drawStats(stats);

		// Draw the menu
		if (pmenu) {

			// Draw the menu

			drawRect((canvasW >> 2) - 8, (canvasH >> 1) - 46, 144, 60, 0);

			for (count = 0; count < 3; count++) {

				if (count == option) fontmn2->mapPalette(240, 8, 31, 16);
				else fontmn2->mapPalette(240, 8, 0, 16);

				fontmn2->showString(options[count], canvasW >> 2, (canvasH >> 1) + (count << 4) - 38);

			}

			fontmn2->restorePalette();

		}

	}

	return E_NONE;

}


