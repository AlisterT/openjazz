
/*
 *
 * levelframe.cpp
 *
 * 19th July 2009: Created levelframe.cpp from parts of level.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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


#include "bullet.h"
#include "event/event.h"
#include "level.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "player/player.h"


int Level::playFrame () {

	Bullet *nextBullet;
	Event *nextEvent;
	int x, y;


	// Search for active events
	for (y = FTOT(viewY) - 5; y < ITOT(FTOI(viewY) + viewH) + 5; y++) {

		for (x = FTOT(viewX) - 5; x < ITOT(FTOI(viewX) + viewW) + 5; x++)
			{

			if ((x >= 0) && (y >= 0) && (x < LW) && (y < LH) &&
				grid[y][x].event && (grid[y][x].event < 121)) {

				nextEvent = firstEvent;

				while (nextEvent) {

					// If the event has been found, stop searching
					if (nextEvent->isFrom(x, y)) break;

					nextEvent = nextEvent->getNext();

				}

				// If the event wasn't found, create it
				if (!nextEvent) firstEvent = new Event(x, y, firstEvent);

			}

		}

	}


	// Determine the players' trajectories
	for (x = 0; x < nPlayers; x++) players[x].control(ticks);


	// Process active events

	pathNode = (ticks >> 5) % pathLength;

	if (firstEvent) {

		if (firstEvent->playFrame(ticks)) {

			nextEvent = firstEvent->getNext();
			delete firstEvent;
			firstEvent = nextEvent;

		}

	}


	// Process bullets

	if (firstBullet) {

		if (firstBullet->playFrame(ticks)) {

			nextBullet = firstBullet->getNext();
			delete firstBullet;
			firstBullet = nextBullet;

		}

	}


	// Apply as much of those trajectories as possible, without going into the
	// scenery
	for (x = 0; x < nPlayers; x++) players[x].move(ticks);



	// Check if time has run out
	if (ticks > endTime) {

		if (!gameMode) {

			if ((difficulty >= 2) && (stage == LS_NORMAL))
				localPlayer->kill(NULL, endTime);

		} else gameMode->outOfTime();

	}


	// Calculate viewport
	if (game && (stage == LS_END)) game->view();
	else localPlayer->view(ticks);

	// Ensure the new viewport is within the level
	if (viewX < 0) viewX = 0;
	if (FTOI(viewX) + viewW >= TTOI(LW)) viewX = ITOF(TTOI(LW) - viewW);
	if (viewY < 0) viewY = 0;
	if (FTOI(viewY) + viewH >= TTOI(LH)) viewY = ITOF(TTOI(LH) - viewH);


	return E_NONE;

}



void Level::draw () {

	GridElement *ge;
	Event *event;
	Bullet *bullet;
	SDL_Rect src, dst;
	int vX, vY;
	int x, y, bgScale;


	// Set tile drawing dimensions
	src.w = TTOI(1);
	src.h = TTOI(1);
	src.x = 0;


	// Use the local player's viewport
	dst.x = 0;
	dst.y = 0;
	vX = FTOI(viewX);
	vY = FTOI(viewY);
	dst.w = viewW;
	dst.h = viewH;
	SDL_SetClipRect(screen, &dst);


	if ((viewW < screenW) || (viewH < screenH)) clearScreen(15);

	// If there is a sky, draw it
	if (sky) {

		// Background scale
		if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
		else bgScale = ((screenH - 34) / 100) + 1;

		for (y = 0; y < viewH; y += bgScale)
			drawRect(0, y, screenW, bgScale, 156 + (y / bgScale));


		// Show sun / moon / etc.
		if (skyOrb) {

			dst.x = (viewW * 4) / 5;
			dst.y = (viewH * 3) / 25;
			src.y = TTOI(skyOrb);
			SDL_BlitSurface(tileSet, &src, screen, &dst);

		}

	} else {

		// If there is no sky, draw a blank background
		// This is only very occasionally actually visible
		clearScreen(127);

	}



	// Show background tiles

	for (y = 0; y <= ITOT(viewH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(viewW - 1) + 1; x++) {

			// Get the grid element from the given coordinates
			ge = grid[y + ITOT(vY)] + x + ITOT(vX);

			// If this tile uses a black background, draw it
			if (ge->bg)
				drawRect(TTOI(x) - (vX & 31), TTOI(y) - (vY & 31), 32, 32,
					BLACK);


			// If this is not a foreground tile, draw it
			if ((eventSet[ge->event][E_BEHAVIOUR] != 38) &&
			    ((ge->event < 124) || (ge->event > 125))  ) {

				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);
				src.y = TTOI(ge->tile);
				SDL_BlitSurface(tileSet, &src, screen, &dst);

			}

		}

	}


	// Show active events
	event = firstEvent;

	while (event) {

		event->draw(ticks);
		event = event->getNext();

	}


	// Show the players

	for (x = 0; x < nPlayers; x++) players[x].draw(ticks);


	// Show bullets
	bullet = firstBullet;

	while (bullet) {

		bullet->draw();
		bullet = bullet->getNext();

	}



	// Show foreground tiles

	for (y = 0; y <= ITOT(viewH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(viewW - 1) + 1; x++) {

			// Get the grid element from the given coordinates
			ge = grid[y + ITOT(vY)] + x + ITOT(vX);

			// If this is an "animated" foreground tile, draw it
			if (ge->event == 123) {

				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);
				if (ticks & 64) src.y = TTOI(eventSet[ge->event][E_YAXIS]);
				else src.y = TTOI(eventSet[ge->event][E_MULTIPURPOSE]);
				SDL_BlitSurface(tileSet, &src, screen, &dst);

			}

			// If this is a foreground tile, draw it
			if ((ge->event == 124) || (ge->event == 125) ||
				(eventSet[ge->event][E_BEHAVIOUR] == 38)   ) {

				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);
				src.y = TTOI(ge->tile);
				SDL_BlitSurface(tileSet, &src, screen, &dst);

			}

		}

	}

	// Uncomment the following for a line showing the water level
/*	drawRect(0, FTOI(getWaterLevel(ticks) - viewY), screenW, 2, 24); */


	SDL_SetClipRect(screen, NULL);


	// Show panel

	// Change the ammo type display on the panel
	dst.x = 250;
	dst.y = 2;
	SDL_BlitSurface(panelAmmo[localPlayer->getAmmo(false) + 1], NULL, panel,
		&dst);

	dst.x = 0;
	dst.y = screenH - 33;
	SDL_BlitSurface(panel, NULL, screen, &dst);
	drawRect(0, screenH - 1, 320, 1, BLACK);


	// Show panel data

	// Show score
	panelSmallFont->showNumber(localPlayer->getScore(), 84, screenH - 27);

	// Show time remaining
	if (endTime > ticks) x = endTime - ticks;
	else x = 0;
	y = x / (60 * 1000);
	panelSmallFont->showNumber(y, 116, screenH - 27);
	x -= (y * 60 * 1000);
	y = x / 1000;
	panelSmallFont->showNumber(y, 136, screenH - 27);
	x -= (y * 1000);
	y = x / 100;
	panelSmallFont->showNumber(y, 148, screenH - 27);

	// Show lives
	panelSmallFont->showNumber(localPlayer->getLives(), 124, screenH - 13);

	// Show planet number

	
	if (worldNum <= 41) // Main game levels
		panelSmallFont->showNumber((worldNum % 3) + 1, 184, screenH - 13);
	else if ((worldNum >= 50) && (worldNum <= 52)) // Christmas levels
		panelSmallFont->showNumber(worldNum - 49, 184, screenH - 13);
	else panelSmallFont->showNumber(worldNum, 184, screenH - 13);

	// Show level number
	panelSmallFont->showNumber(levelNum + 1, 196, screenH - 13);

	// Show ammo
	if (localPlayer->getAmmo(false) == -1)
		panelSmallFont->showString(":;", 225, screenH - 13);
	else panelSmallFont->showNumber(localPlayer->getAmmo(true), 245,
		screenH - 13);


	// Draw the health bar

	dst.x = 20;
	x = localPlayer->getEnergy();

	if (FTOI(energyBar) < (x << 4)) {

		if ((x << 14) - energyBar < mspf * 40) energyBar = x << 14;
		else energyBar += mspf * 40;

	} else if (FTOI(energyBar) > (x << 4)) {

		if (energyBar - (x << 14) < mspf * 40) energyBar = x << 14;
		else energyBar -= mspf * 40;

	}

	if (energyBar > F1) {

		dst.w = FTOI(energyBar) - 1;

		// Choose energy bar colour
		if (x == 4) x = 24;
		else if (x == 3) x = 17;
		else if (x == 2) x = 80;
		else if (x <= 1) x = 32 + (((ticks / 75) * 4) & 15);

		// Draw energy bar
		drawRect(dst.x, screenH - 13, dst.w, 7, x);

		dst.x += dst.w;
		dst.w = 64 - dst.w;

	} else dst.w = 64;


	// Fill in remaining energy bar space with black
	drawRect(dst.x, screenH - 13, dst.w, 7, BLACK);


	return;

}


