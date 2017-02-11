
/**
 *
 * @file jj1demolevel.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 22nd July 2008: Created levelload.c from parts of level.c
 * 3rd February 2009: Renamed level.c to level.cpp and levelload.c to
 *                    levelload.cpp
 * 18th July 2009: Created demolevel.cpp from parts of level.cpp and
 *                 levelload.cpp
 * 1st August 2012: Renamed demolevel.cpp to jj1demolevel.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the loading and playing of demo levels.
 *
 */


#include "jj1level.h"
#include "jj1levelplayer/jj1levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "loop.h"
#include "util.h"


/**
 * Create a JJ1 demo level.
 *
 * @param owner The current game
 * @param fileName Name of the file containing the macro data.
 */
JJ1DemoLevel::JJ1DemoLevel (Game* owner, const char* fileName) : JJ1Level(owner) {

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

	ret = load(levelFile, false);

	delete[] levelFile;

	if (ret < 0) throw ret;

	return;

}


/**
 * Delete the JJ1 demo level.
 */
JJ1DemoLevel::~JJ1DemoLevel () {

	delete[] macro;

	return;

}


/**
 * Play the demo.
 *
 * @return Error code
 */
int JJ1DemoLevel::play () {

	unsigned char macroPoint;
	int ret;


	tickOffset = globalTicks;
	ticks = 17;
	steps = 0;

	video.setPalette(palette);

	while (true) {

		// Do general processing
		if (::loop(NORMAL_LOOP, paletteEffects) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE) || controls.release(C_ENTER)) return E_NONE;

		if (controls.release(C_STATS)) stats ^= S_SCREEN;


		timeCalcs();



		// Use macro

		macroPoint = macro[(ticks / 76) & 1023];

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
		while (getTimeChange() >= T_STEP) {

			ret = step();
			steps++;

			if (ret < 0) return ret;

		}


		// Handle player reactions
		if (localPlayer->getJJ1LevelPlayer()->reacted(ticks) == PR_KILLED) return LOST;


		// Draw the graphics

		draw();
		drawOverlay(LEVEL_BLACK, false, 0, 0, 0, 0);


		font->showString("demo", (canvasW >> 1) - 36, 32);


	}

	return E_NONE;

}


