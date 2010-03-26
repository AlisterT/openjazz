
/*
 *
 * demolevel.cpp
 *
 * 18th July 2009: Created demolevel.cpp from parts of level.cpp and
 *                 levelload.cpp
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
 * Deals with the loading and playing of demo levels.
 *
 */


#include "level.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "player/player.h"


DemoLevel::DemoLevel (const char *fileName) {

	File *file;
	char *levelFile;
	int lNum, wNum, diff, ret;

	gameMode = NULL;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	// Check this is a normal level
	if (file->loadShort() == 0) throw E_DEMOTYPE;

	// Level file to load
	lNum = file->loadShort();
	wNum = file->loadShort();
	levelFile = createFileName(F_LEVEL, lNum, wNum);

	// Difficulty
	diff = file->loadShort();

	macro = file->loadBlock(1024);

	// Load level data

	ret = load(levelFile, diff, false);

	delete[] levelFile;

	if (ret < 0) throw ret;

	return;

}


DemoLevel::~DemoLevel () {

	delete[] macro;

	return;

}


int DemoLevel::play () {

	int stats;
	unsigned char macroPoint;
	int ret;


	// Arbitrary initial value
	smoothfps = 50.0f;

	tickOffset = globalTicks;
	ticks = 16;
	prevStepTicks = 0;

	stats = S_NONE;

	while (true) {

		// Do general processing
		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_STATS)) stats ^= S_SCREEN;


		timeCalcs(false);



		// Use macro

		macroPoint = macro[(ticks / 90) % 1024];

		if (macroPoint & 128) return E_NONE;

		if (macroPoint & 1) {

			localPlayer->setControl(C_LEFT, false);
			localPlayer->setControl(C_RIGHT, false);
			localPlayer->setControl(C_UP, !(macroPoint & 4));

		} else {

			localPlayer->setControl(C_LEFT, !(macroPoint & 2));
			localPlayer->setControl(C_RIGHT, macroPoint & 2);
			localPlayer->setControl(C_UP, false);

		}

		localPlayer->setControl(C_DOWN, macroPoint & 8);
		localPlayer->setControl(C_FIRE, macroPoint & 16);
		localPlayer->setControl(C_CHANGE, macroPoint & 32);
		localPlayer->setControl(C_JUMP, macroPoint & 64);
		localPlayer->setControl(C_SWIM, macroPoint & 64);



		// Check if level has been won
		if (getStage() == LS_END) return WON;


		// Process frame-by-frame activity

		// Process step
		if (ticks >= prevStepTicks + 16) {

			ret = step();

			if (ret < 0) return ret;

		}


		// Handle player reactions
		if (localPlayer->reacted(ticks) == PR_KILLED) return LOST;


		// Draw the graphics

		draw();


		fontmn1->showString("DEMO", (canvasW >> 1) - 36, 32);


		// Draw graphics statistics

		if (stats & S_SCREEN) {

			drawRect(236, 9, 80, 32, BLACK);

			panelBigFont->showNumber(canvasW, 268, 15);
			panelBigFont->showString("x", 272, 15);
			panelBigFont->showNumber(canvasH, 308, 15);
			panelBigFont->showString("fps", 244, 27);
			panelBigFont->showNumber((int)smoothfps, 308, 27);

		}

	}

	return E_NONE;

}


