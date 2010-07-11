
/*
 *
 * jj2level.cpp
 *
 * 29th June 2010: Created jj2level.cpp from parts of level.cpp
 * 2nd July 2010: Created jj2event.cpp from parts of jj2level.cpp
 * 2nd July 2010: Created jj2eventframe.cpp from parts of jj2level.cpp
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
 * Deals with the creating, playing and freeing of JJ2 levels.
 *
 */


#include "jj2event/jj2event.h"
#include "jj2level.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/jj2levelplayer.h"
#include "scene/scene.h"
#include "util.h"

#include <string.h>


JJ2Level::JJ2Level (char* fileName, unsigned char diff, bool checkpoint, bool multi) {

	int ret;

	// Load level data

	ret = load(fileName, diff, checkpoint);

	if (ret < 0) throw ret;

	multiplayer = multi;

	return;

}


JJ2Level::~JJ2Level () {

	int count;

	if (events) delete events;
	delete[] *mods;
	delete[] mods;

	for (count = 0; count < LAYERS; count++) delete layers[count];

	delete[] flippedMask;
	delete[] mask;

	delete[] musicFile;
	delete[] nextLevel;

	for (count = 0; count < nAnimSets; count++) {

		if (animSets[count]) delete[] animSets[count];

	}

	delete[] animSets;
	delete[] spriteSet;

	SDL_FreeSurface(flippedTileSet);
	SDL_FreeSurface(tileSet);

	delete font;

	// Restore panel font palette
	panelBigFont->restorePalette();
	panelSmallFont->restorePalette();

	return;

}


bool JJ2Level::checkMaskUp (fixed x, fixed y) {

	int tX, tY;

	tX = FTOT(x);
	tY = FTOT(y);

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (tX >= layer->getWidth()) || (tY >= layer->getHeight()))
		return true;

	// Event 1 is one-way
	// Event 3 is vine
	// Event 4 is hook
	if ((mods[tY][tX].type == 1) || (mods[tY][tX].type == 3) || (mods[tY][tX].type == 4)) return false;

	// Check the mask in the tile in question
	return (layer->getFlipped(tX, tY)? flippedMask: mask)[(layer->getTile(tX, tY) << 10) + ((y >> 5) & 992) + ((x >> 10) & 31)];

}


bool JJ2Level::checkMaskDown (fixed x, fixed y, bool drop) {

	int tX, tY;

	tX = FTOT(x);
	tY = FTOT(y);

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (tX >= layer->getWidth()) || (tY >= layer->getHeight()))
		return true;

	// Event 3 is vine
	// Event 4 is hook
	if (drop && ((mods[tY][tX].type == 3) || (mods[tY][tX].type == 4))) return false;

	// Check the mask in the tile in question
	return (layer->getFlipped(tX, tY)? flippedMask: mask)[(layer->getTile(tX, tY) << 10) + ((y >> 5) & 992) + ((x >> 10) & 31)];

}


void JJ2Level::setNext (char* fileName) {

	unsigned char buffer[MTL_L_PROP];

	delete[] nextLevel;
	nextLevel = createString(fileName);

	if (multiplayer) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 0; // set next level
		buffer[3] = 0;
		buffer[4] = 0;

		game->send(buffer);

	}

	return;

}


void JJ2Level::setFrame (unsigned char gridX, unsigned char gridY, unsigned char frame) {

	unsigned char buffer[MTL_L_GRID];

	layer->setFrame(gridX, gridY, frame);

	if (multiplayer) {

		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 0; // tile variable
		buffer[5] = frame;

		game->send(buffer);

	}

	return;

}


JJ2Modifier* JJ2Level::getModifier (unsigned char gridX, unsigned char gridY) {

	return mods[gridY] + gridX;

}


Sprite* JJ2Level::getSprite (unsigned char sprite) {

	return spriteSet + sprite;

}


Anim* JJ2Level::getAnim (int set, int anim, bool flipped) {

	return (flipped? flippedAnimSets: animSets)[set] + anim;

}


void JJ2Level::setWaterLevel (unsigned char gridY, bool instant) {

	unsigned char buffer[MTL_L_PROP];

	waterLevelTarget = TTOF(gridY);

	if (instant) waterLevel = waterLevelTarget - F8;

	if (multiplayer) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 1; // set water level
		buffer[3] = gridY;
		buffer[4] = 0; // Doesn't really matter

		game->send(buffer);

	}

	return;

}


fixed JJ2Level::getWaterLevel () {

	return waterLevel;

}


void JJ2Level::warp (JJ2LevelPlayer *player, int id) {

	int x, y;

	for (y = 0; y < layer->getHeight(); y++) {

		for (x = 0; x < layer->getWidth(); x++) {

			if ((mods[y][x].type == 240) && ((mods[y][x].properties & 255) == id)) {

				player->setPosition(TTOF(x), TTOF(y));

				return;

			}

		}

	}

}


void JJ2Level::receive (unsigned char* buffer) {

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_L_PROP:

			if (buffer[2] == 1) {

				waterLevelTarget = TTOF(buffer[3]);

			} else if (buffer[2] == 2) {

				if (stage == LS_NORMAL)
					endTime += 2 * 60 * 1000; // 2 minutes. Is this right?

			}

			break;

		case MT_L_GRID:

			if (buffer[4] == 0) layer->setFrame(buffer[2], buffer[3], buffer[5]);

			break;

		case MT_L_STAGE:

			stage = LevelStage(buffer[2]);

			break;

	}

	return;

}


int JJ2Level::play () {

	JJ2LevelPlayer* jj2LevelPlayer;
	const char* options[5] =
		{"continue game", "save game", "load game", "setup options", "quit game"};
	bool pmessage, pmenu;
	int option;
	unsigned int returnTime;
 	int count;


	jj2LevelPlayer = localPlayer->getJJ2LevelPlayer();

	tickOffset = globalTicks;
	ticks = 16;
	prevStepTicks = 0;

	pmessage = pmenu = false;
	option = 0;

	returnTime = 0;

	video.setPalette(palette);

	playMusic(musicFile);

	while (true) {

		count = loop(pmenu, option, pmessage);

		if (count <= 0) return count;


		// Check if level has been won
		if (game && returnTime && (ticks > returnTime)) {

			count = game->setLevel(nextLevel);

			if (count < 0) return count;

			return WON;

		}


		// Process frame-by-frame activity

		if (!paused && (ticks >= prevStepTicks + 16)) {

			// Apply controls to local player
			for (count = 0; count < PCONTROLS; count++)
				localPlayer->setControl(count, controls.getState(count));

			count = step();

			if (count) return count;

		}


		// Draw the graphics

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			font->showString("pause", (canvasW >> 1) - 44, 32);


		// Draw statistics
		drawStats(JJ2_BLACK);


		if (stage == LS_END) {

			// The level is over, so draw gem counts

			if (!returnTime) {

				returnTime = ticks + 3000;
				playSound(S_UPLOOP);

			}

			// Display statistics

			font->showString("red gems", (canvasW >> 1) - 152, (canvasH >> 1) - 60);
			font->showNumber(jj2LevelPlayer->getGems(0), (canvasW >> 1) + 124, (canvasH >> 1) - 60);

			font->showString("green gems", (canvasW >> 1) - 152, (canvasH >> 1) - 40);
			font->showNumber(jj2LevelPlayer->getGems(1), (canvasW >> 1) + 124, (canvasH >> 1) - 40);

			font->showString("blue gems", (canvasW >> 1) - 152, (canvasH >> 1) - 20);
			font->showNumber(jj2LevelPlayer->getGems(2), (canvasW >> 1) + 124, (canvasH >> 1) - 20);

		}


		if (pmenu) {

			// Draw the menu

			drawRect((canvasW >> 2) - 8, (canvasH >> 1) - 46, 144, 92, JJ2_BLACK);

			for (count = 0; count < 5; count++) {

				if (count == option) fontmn2->mapPalette(240, 8, 31, -8);
				else fontmn2->mapPalette(240, 8, 71, -8);

				fontmn2->showString(options[count], canvasW >> 2, (canvasH >> 1) + (count << 4) - 38);

			}

			fontmn2->restorePalette();

		}

	}

	return E_NONE;

}


