
/*
 *
 * baselevel.cpp
 *
 * 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
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
 * Deals with functionality common to ordinary levels and bonus levels.
 *
 */


#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "player/player.h"
#include "baselevel.h"


BaseLevel::BaseLevel () {

	// Arbitrary initial value
	smoothfps = 50.0f;

	return;

}


void BaseLevel::timeCalcs (bool paused) {

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


void BaseLevel::drawStats (int stats) {

	int count, width;

	// Draw graphics statistics

	if (stats & S_SCREEN) {

#ifdef SCALE
		if (scaleFactor > 1)
			drawRect(canvasW - 84, 11, 80, 37, BLACK);
		else
#endif
			drawRect(canvasW - 84, 11, 80, 25, BLACK);

		panelBigFont->showNumber(screenW, canvasW - 52, 14);
		panelBigFont->showString("x", canvasW - 48, 14);
		panelBigFont->showNumber(screenH, canvasW - 12, 14);
		panelBigFont->showString("fps", canvasW - 76, 26);
		panelBigFont->showNumber((int)smoothfps, canvasW - 12, 26);

#ifdef SCALE
		if (scaleFactor > 1) {

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

		drawRect((canvasW >> 1) - 48, 11, width + 57, (nPlayers * 12) + 1, BLACK);

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

