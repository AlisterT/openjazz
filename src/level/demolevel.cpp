
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
#include "player/levelplayer.h"
#include "loop.h"
#include "util.h"


DemoLevel::DemoLevel (const char* fileName) {

	File* file;
	char* levelFile;
	int lNum, wNum, diff, ret;

	multiplayer = false;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	// Check this is a normal level
	if (file->loadShort() == 0) throw E_DEMOTYPE;

	// Level file to load
	lNum = file->loadShort(9);
	wNum = file->loadShort(999);
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

	unsigned char macroPoint;
	int ret;


	tickOffset = globalTicks;
	ticks = 16;
	prevStepTicks = 0;

	video.setPalette(palette);

	while (true) {

		// Do general processing
		if (::loop(NORMAL_LOOP, paletteEffects) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		if (controls.release(C_STATS)) stats ^= S_SCREEN;


		timeCalcs();



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
		if (localPlayer->getLevelPlayer()->reacted(ticks) == PR_KILLED) return LOST;


		// Draw the graphics

		draw();
		drawStats(LEVEL_BLACK);


		font->showString("demo", (canvasW >> 1) - 36, 32);


	}

	return E_NONE;

}


