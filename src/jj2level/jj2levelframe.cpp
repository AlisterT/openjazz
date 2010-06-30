
/*
 *
 * jj2levelframe.cpp
 *
 * 29th June 2010: Created jj2levelframe.cpp from parts of levelframe.cpp
 * 30th June 2010: Created jj2layer.cpp from parts of jj2levelframe.cpp
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
 * Provides the once-per-frame functions for levels.
 *
 */


#include "jj2level.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "player/jj2levelplayer.h"
#include "util.h"


int JJ2Level::step () {

	int x;
	int msps;


	// Milliseconds per step
	msps = ticks - prevStepTicks;
	prevStepTicks = ticks;


	// Determine the players' trajectories
	for (x = 0; x < nPlayers; x++) players[x].getJJ2LevelPlayer()->control(ticks, msps);


	// Apply as much of those trajectories as possible, without going into the
	// scenery
	for (x = 0; x < nPlayers; x++) players[x].getJJ2LevelPlayer()->move(ticks, msps);



	// Handle change in water level
	if (waterLevel < waterLevelTarget) waterLevelSpeed += 100 * msps;
	else waterLevelSpeed -= 100 * msps;
	if (waterLevelSpeed > 40000) waterLevelSpeed = 40000;
	if (waterLevelSpeed < -40000) waterLevelSpeed = -40000;

	waterLevel += (waterLevelSpeed * msps) >> 10;


	// Handle player reactions
	for (x = 0; x < nPlayers; x++) {

		if (players[x].getJJ2LevelPlayer()->reacted(ticks) == JJ2PR_KILLED) {

			if (!gameMode) return LOST;

			game->resetPlayer(players + x);

		}

	}


	return E_NONE;

}


void JJ2Level::draw () {

	int count, energy;
	unsigned int change;


	// Calculate viewport
	if (game && (stage == LS_END)) game->view(paused? 0: ((ticks - prevTicks) * 160));
	else localPlayer->getJJ2LevelPlayer()->view(ticks, paused? 0: (ticks - prevTicks));

	// Ensure the new viewport is within the level
	if (viewX < 0) viewX = 0;
	if (FTOI(viewX) + viewW >= TTOI(layers[3]->getWidth())) viewX = ITOF(TTOI(layers[3]->getWidth()) - viewW);
	if (viewY < 0) viewY = 0;
	if (FTOI(viewY) + viewH >= TTOI(layers[3]->getHeight())) viewY = ITOF(TTOI(layers[3]->getHeight()) - viewH);


	// Show background layers
	for (count = 7; count >= 3; count--) layers[count]->draw(tileSet);


	// Calculate change since last step
	change = paused? 0: ticks - prevStepTicks;


	// Show the players
	for (count = 0; count < nPlayers; count++) players[count].getJJ2LevelPlayer()->draw(ticks, change);


	// Show foreground layers
	for (count = 2; count >= 0; count--) layers[count]->draw(tileSet);


	// Temporary lines showing the water level
	drawRect(0, FTOI(waterLevel - viewY), canvasW, 2, 72);
	drawRect(0, FTOI(waterLevel - viewY) + 3, canvasW, 1, 72);
	drawRect(0, FTOI(waterLevel - viewY) + 6, canvasW, 1, 72);
	drawRect(0, FTOI(waterLevel - viewY) + 10, canvasW, 1, 72);


	// Show "panel" data

	// Show score
	if (gameMode) gameMode->drawScore(font);
	else panelSmallFont->showNumber(localPlayer->getScore(), 16, 8);


	// Draw hearts

	energy = localPlayer->getJJ2LevelPlayer()->getEnergy();

	for (count = 1; count <= energy; count++) {

		drawRect(viewW - (count * 12), 4, 8, 8, 48);

	}


	// Show lives
	panelSmallFont->showNumber(localPlayer->getLives(), 16, canvasH - 16);


	// Show ammo
	if (localPlayer->getAmmo(false) == -1) {

		panelSmallFont->showString(":", viewW - 24, canvasH - 16);
		panelSmallFont->showString(";", viewW - 16, canvasH - 16);

	} else panelSmallFont->showNumber(localPlayer->getAmmo(true), viewW - 8, canvasH - 16);


	return;

}


