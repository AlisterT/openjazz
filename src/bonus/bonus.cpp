
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

#ifdef __SYMBIAN32__
extern float cosf (float);
extern float sinf (float);
extern float tanf (float);
#endif

#define PI 3.141592f


int Bonus::loadTiles (char *fileName) {

	File *file;
	unsigned char *pixels;
	unsigned char *sorted;
	int count, x, y;

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
	pixels = file->loadRLE(1024 * 60);
	tileSet = createSurface(pixels, 32, 32 * 60);

	// Create mask
	for (count = 0; count < 60; count++) {

		memset(mask[count], 0, 64);

		for (y = 0; y < 32; y++) {

			for (x = 0; x < 32; x++) {

				if ((pixels[(count << 10) + (y << 5) + x] & 240) == 192)
					mask[count][((y << 1) & 56) + ((x >> 2) & 7)] = 1;

			}

		}

	}

	delete[] pixels;

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

			grid[y][x].tile = buffer[x + (y * BLW)];

			if (grid[y][x].tile > 59) grid[y][x].tile = 59;

		}

	}

	delete[] buffer;


	file->skipRLE(); // Mysterious block


	// Load events

	buffer = file->loadRLE(BLW * BLH);

	for (y = 0; y < BLW; y++) {

		for (x = 0; x < BLH; x++) {

			grid[y][x].event = buffer[x + (y * BLW)];

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
	if (paletteEffects) delete paletteEffects;

	// Spinny whirly thing
	paletteEffects = new RotatePaletteEffect(112, 16, F32, NULL);

	// Track sides
	paletteEffects = new RotatePaletteEffect(192, 16, F32, paletteEffects);

	// Bouncy things
	paletteEffects = new RotatePaletteEffect(240, 16, F32, paletteEffects);


	// Adjust fontsFont to use bonus level palette
	fontsFont->mapPalette(0, 32, 15, -16);


	return;

}


Bonus::~Bonus () {

	// Free the palette effects
	if (paletteEffects) {

		delete paletteEffects;
		paletteEffects = NULL;

	}

	SDL_FreeSurface(tileSet);

	fontsFont->restorePalette();

	return;

}


bool Bonus::isEvent (fixed x, fixed y) {

	return ((x & 32767) > F12) && ((x & 32767) < F20) &&
		((y & 32767) > F12) && ((y & 32767) < F20);

}


bool Bonus::checkMask (fixed x, fixed y) {

	BonusGridElement *ge;

	ge = grid[FTOT(y) & 255] + (FTOT(x) & 255);

	// Hand
	if ((ge->event == 3) && isEvent(x, y)) return true;

	// Check the mask in the tile in question
	return mask[ge->tile][((y >> 9) & 56) + ((x >> 12) & 7)];

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

		playerX = players[count].getX();
		playerY = players[count].getY();

		players[count].bonusStep(ticks, msps, this);

		if (isEvent(playerX, playerY)) {

			gridX = FTOT(playerX) & 255;
			gridY = FTOT(playerY) & 255;

			switch (grid[gridY][gridX].event) {

				case 1: // Extra time

					addTimer();
					grid[gridY][gridX].event = 0;

					break;

				case 2: // Gem

					players[count].addItem();
					grid[gridY][gridX].event = 0;

					if (players[count].getItems() >= items) {

						players[count].addLife();

						return WON;

					}

					break;

				case 4: // Exit

					return LOST;

				default:

					// Do nothing

					break;

			}

		}

	}

	direction = localPlayer->getDirection();

	return E_NONE;

}


void Bonus::draw () {

	SDL_Rect dst;
	fixed playerX, playerY, playerSin, playerCos;
	fixed distance, opposite, adjacent;
	int levelX, levelY;
	int spriteW, spriteH;
	int x, y;


	// Draw the background

	for (x = -(localPlayer->getDirection() & 1023); x < canvasW; x += background->w) {

		dst.x = x;
		dst.y = (canvasH >> 1) - 4;
		SDL_BlitSurface(background, NULL, canvas, &dst);

	}

	x = 171;

	for (y = (canvasH >> 1) - 5; (y >= 0) && (x > 128); y--) drawRect(0, y, canvasW, 1, x--);

	if (y > 0) drawRect(0, 0, canvasW, y + 1, 128);


	// Draw the ground

	playerX = localPlayer->getX();
	playerY = localPlayer->getY();
	playerSin = fixed(1024.0f * sinf(PI * float(direction) / 512.0f));
	playerCos = fixed(1024.0f * cosf(PI * float(direction) / 512.0f));

	if (SDL_MUSTLOCK(canvas)) SDL_LockSurface(canvas);

	for (y = 1; y <= (canvasH >> 1) - 15; y++) {

		distance = fixed(1024.0f * tanf((float(y) / float(canvasH >> 1)) + ((PI / 2.0f) - 1.0f)) * 20.0f);
		opposite = MUL(distance, playerSin);
		adjacent = MUL(distance, playerCos);

		for (x = 0; x < canvasW; x++) {

			fixed nX = ITOF(x - (canvasW >> 1)) / canvasW;

			levelX = FTOI(playerX + opposite + MUL(nX, adjacent));
			levelY = FTOI(playerY - adjacent + MUL(nX, opposite));

			((unsigned char *)(canvas->pixels))[(canvas->pitch * (canvasH - y)) + x] =
				((unsigned char *)(tileSet->pixels))
					[(grid[ITOT(levelY) & 255][ITOT(levelX) & 255].tile * 1024) +
						((levelY & 31) * tileSet->pitch) + (levelX & 31)];

		}

	}

	if (SDL_MUSTLOCK(canvas)) SDL_UnlockSurface(canvas);


	// Draw the events

	for (y = -7; y < 8; y++) {

		fixed sY = TTOF(y) + F16 - (playerY & 32767);

		for (x = -7; x < 8; x++) {

			fixed sX = TTOF(x) + F16 - (playerX & 32767);

			fixed divisor = MUL(sX, playerSin) - MUL(sY, playerCos);

			if (FTOI(divisor) > 0) {

				spriteW = 1000 / FTOI(divisor);
				spriteH = 1000 / FTOI(divisor);

				fixed nX = DIV(MUL(sX, playerCos) + MUL(sY, playerSin), divisor);

				dst.x = FTOI(nX * canvasW) + ((canvasW - spriteW) >> 1);
				dst.y = (canvasH - spriteH) >> 1;

				switch (grid[(y + FTOT(playerY)) & 255][(x + FTOT(playerX)) & 255].event) {

					case 0: // No event

						break;

					case 1: // Extra time

						drawRect(dst.x, dst.y, spriteW, spriteH, 60);

						break;

					case 2: // Gem

						drawRect(dst.x, dst.y, spriteW, spriteH, 67);

						break;

					case 3: // Hand

						drawRect(dst.x, dst.y, spriteW, spriteH, 15);

						break;

					case 4: // Exit

						drawRect(dst.x, dst.y, spriteW, spriteH, 45);

						break;

					default:

						drawRect(dst.x, dst.y, spriteW, spriteH, 0);

						break;

				}

			}

		}

	}


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
	stats = 0;

	returnTime = 0;

	video.setPalette(palette);

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
							levelPE = paletteEffects;
							paletteEffects = NULL;

							if (menu->setup() == E_QUIT) return E_QUIT;

							// Restore level palette
							video.setPalette(palette);

							// Restore palette effects
							paletteEffects = levelPE;

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
				paletteEffects = new WhiteOutPaletteEffect(T_BONUS_END, paletteEffects);
				returnTime = ticks + T_BONUS_END;

			}

		}


		// Draw the graphics

		if (ticks < returnTime) direction += (ticks - prevTicks) * T_BONUS_END / (returnTime - ticks);

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			fontsFont->showString("pause", (canvasW >> 1) - 44, 32);

		// Draw statistics
		drawStats(stats, 0);

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


