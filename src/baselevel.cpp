
/**
 *
 * @file baselevel.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2011 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with functionality common to ordinary levels and bonus levels.
 *
 */


#include "game/game.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "menu/menu.h"
#include "player/player.h"
#include "scene/scene.h"
#include "baselevel.h"
#include "loop.h"


/**
 * Create a new base level
 */
BaseLevel::BaseLevel () {

	// Arbitrary initial value
	smoothfps = 50.0f;

	paletteEffects = NULL;

	paused = false;

	// Set the level stage
	stage = LS_NORMAL;

	stats = 0;

	return;

}


/**
 * Destroy base level
 */
BaseLevel::~BaseLevel () {

	stopMusic();

	if (paletteEffects) delete paletteEffects;

	return;

}


/**
 * Play a cutscene.
 *
 * @param file File name of the cutscene to be played
 *
 * @return Error code
 */
int BaseLevel::playScene (char* file) {

	Scene* scene;
	int ret;

	delete paletteEffects;
	paletteEffects = NULL;

	try {

		scene = new Scene(file);

	} catch (int e) {

		return e;

	}

	ret = scene->play();

	delete scene;

	return ret;

}


/**
 * Perform timing calculations.
 */
void BaseLevel::timeCalcs () {

	// Calculate smoothed fps
	smoothfps = smoothfps + 1.0f -
		(smoothfps * ((float)(ticks - prevTicks)) / 1000.0f);
	/* This equation is a simplified version of
	(fps * c) + (smoothfps * (1 - c))
	where c = (1 / fps)
	and fps = 1000 / (ticks - prevTicks)
	In other words, the response of smoothFPS to changes in FPS decreases as the
	framerate increases
	The following version is for c = (1 / smoothfps)
	*/
	// smoothfps = (fps / smoothfps) + smoothfps - 1;

	// Ignore outlandish values
	if (smoothfps > 9999.0f) smoothfps = 9999.0f;
	if (smoothfps < 1.0f) smoothfps = 1.0f;


	// Track number of ticks of gameplay since the level started

	if (paused) {

		tickOffset = globalTicks - ticks;

	} else if (globalTicks - tickOffset > ticks + 100) {

		prevTicks = ticks;
		ticks += 100;

		tickOffset = globalTicks - ticks;

	} else {

		prevTicks = ticks;
		ticks = globalTicks - tickOffset;

	}

	return;

}


/**
 * Calculate the amount of time since the last completed step.
 *
 * @return Time since last step
 */
int BaseLevel::getTimeChange () {

	return paused? 0: ticks - ((steps * 50) / 3);

}


/**
 * Display on-screen statistics.
 *
 * @param bg Palette index of the statistics box(es)
 */
void BaseLevel::drawStats (unsigned char bg) {

	int count, width;

	// Draw graphics statistics

	if (stats & S_SCREEN) {

#ifdef SCALE
		if (video.getScaleFactor() > 1)
			drawRect(canvasW - 84, 11, 80, 37, bg);
		else
#endif
			drawRect(canvasW - 84, 11, 80, 25, bg);

		panelBigFont->showNumber(video.getWidth(), canvasW - 52, 14);
		panelBigFont->showString("x", canvasW - 48, 14);
		panelBigFont->showNumber(video.getHeight(), canvasW - 12, 14);
		panelBigFont->showString("fps", canvasW - 76, 26);
		panelBigFont->showNumber((int)smoothfps, canvasW - 12, 26);

#ifdef SCALE
		if (video.getScaleFactor() > 1) {

			panelBigFont->showNumber(canvasW, canvasW - 52, 38);
			panelBigFont->showString("x", canvasW - 48, 39);
			panelBigFont->showNumber(canvasH, canvasW - 12, 38);

		}
#endif

	}

	// Draw player list

	if (stats & S_PLAYERS) {

		width = 39;

		for (count = 0; count < nPlayers; count++)
			if (panelBigFont->getStringWidth(players[count].getName()) > width)
				width = panelBigFont->getStringWidth(players[count].getName());

		drawRect((canvasW >> 1) - 48, 11, width + 57, (nPlayers * 12) + 1, bg);

		for (count = 0; count < nPlayers; count++) {

			panelBigFont->showNumber(count + 1,
				(canvasW >> 1) - 24, 14 + (count * 12));
			panelBigFont->showString(players[count].getName(),
				(canvasW >> 1) - 16, 14 + (count * 12));
			panelBigFont->showNumber(players[count].teamScore,
				(canvasW >> 1) + width + 1, 14 + (count * 12));

		}

	}

	return;

}


/**
 * Process in-game menu selection.
 *
 * @param option Chosen menu option
 *
 * @return Error code
 */
int BaseLevel::select (bool& menu, int option) {

	switch (option) {

		case 0: // Continue

			menu = false;

		case 1: // Save

			break;

		case 2: // Load

			break;

		case 3: // Setup

			if (!multiplayer) {

				if (setupMenu.setupMain() == E_QUIT) return E_QUIT;

				// Restore level palette
				video.setPalette(palette);

			}

			break;

		case 4: // Quit game

			return E_NONE;

	}

	return 1;

}

/**
 * Process iteration.
 *
 * @param menu Whether or not the level menu should be displayed
 * @param option Selected menu uption
 * @param message Whether or not the "paused" message is being displayed
 *
 * @return Error code
 */
int BaseLevel::loop (bool& menu, int& option, bool& message) {

	int ret, x, y;

	// Networking
	if (multiplayer) {

		ret = game->step(ticks);

		switch (ret) {

			case E_RETURN:

				return E_NONE;

			case E_NONE:

				break;

			default:

				return ret;

		}

	}


	// Main loop
	if (::loop(NORMAL_LOOP, paletteEffects) == E_QUIT) return E_QUIT;


	if (controls.release(C_ESCAPE)) {

		menu = !menu;
		option = 0;

	}

	if (controls.release(C_PAUSE)) message = !message;

	if (controls.release(C_STATS)) {

		if (!multiplayer) stats ^= S_SCREEN;
		else stats = (stats + 1) & 3;

	}

	if (menu) {

		// Deal with menu controls

		if (controls.release(C_UP)) option = (option + 4) % 5;

		if (controls.release(C_DOWN)) option = (option + 1) % 5;

		if (controls.release(C_ENTER)) {

			ret = select(menu, option);

			if (ret <= 0) return ret;

		}

		if (controls.releaseCursor(x, y)) {

			x -= canvasW >> 2;
			y -= (canvasH >> 1) - 38;

			if ((x >= 0) && (x < 128) && (y >= 0) && (y < 80)) {

				option = y >> 4;

				ret = select(menu, option);

				if (ret <= 0) return ret;

			}

		}

	} else {

		if (controls.releaseCursor(x, y)) menu = true;

	}

	if (!multiplayer) paused = message || menu;

	timeCalcs();

	return 1;

}


/**
 * Add extra time.
 */
void BaseLevel::addTimer () {

	unsigned char buffer[MTL_L_PROP];

	if (stage != LS_NORMAL) return;

	endTime += 2 * 60 * 1000;

	if (endTime >= ticks + (10 * 60 * 1000))
		endTime = ticks + (10 * 60 * 1000) - 1;

	if (multiplayer) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 2; // add timer
		buffer[3] = 0;
		buffer[4] = 0; // Don't really matter

		game->send(buffer);

	}

	return;

}


/**
 * Set the level stage.
 *
 * @param newStage New level stage
 */
void BaseLevel::setStage (LevelStage newStage) {

	unsigned char buffer[MTL_L_STAGE];

	if (stage == newStage) return;

	stage = newStage;

	if (multiplayer) {

		buffer[0] = MTL_L_STAGE;
		buffer[1] = MT_L_STAGE;
		buffer[2] = stage;
		game->send(buffer);

	}

	return;

}


/**
 * Determine the current level stage.
 *
 * @return The current level stage.
 */
LevelStage BaseLevel::getStage () {

	return stage;

}

