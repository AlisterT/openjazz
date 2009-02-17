
/*
 *
 * level.cpp
 *
 * Created as level.c on the 23rd of August 2005
 * Renamed level.cpp on the 3rd of February 2009
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
 * Deals with the running and freeing of ordinary levels.
 *
 */


#include "OpenJazz.h"
#include <string.h>


int Level::checkMask (fixed x, fixed y) {

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x > (LW * TW << 10)) || (y > (LH * TH << 10)))
		return 1;

	// Event 122 is one-way
	if (grid[y >> 15][x >> 15].event == 122) return 0;

	// Check the mask in the tile in question
	return mask[grid[y >> 15][x >> 15].tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


int Level::checkMaskDown (fixed x, fixed y) {

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x > (LW * TW << 10)) || (y > (LH * TH << 10)))
		return 1;

	// Check the mask in the tile in question
	return mask[grid[y >> 15][x >> 15].tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}



int Level::playFrame (int ticks) {

	Event *nextEvent;
	Bullet *nextBullet;
	int x, y;


	// Search for active events
	for (y = (localPlayer->viewY >> 15) - 5;
		y < (((localPlayer->viewY >> 10) + localPlayer->viewH) >> 5) + 5; y++) {

		for (x = (localPlayer->viewX >> 15) - 5;
			x < (((localPlayer->viewX >> 10) + localPlayer->viewW) >> 5) + 5;
			x++) {

			if ((x >= 0) && (y >= 0) && (x < LW) && (y < LH) &&
				grid[y][x].event && (grid[y][x].event < 122)) {

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


	// Apply controls to local player
	for (x = 0; x < PCONTROLS; x++)
		localPlayer->setControl(x, controls[x].state == SDL_PRESSED);

	// Determine the player's trajectory
	localPlayer->control(ticks);


	// Process active events

	pathNode = (ticks >> 5) % pathLength;

	if (firstEvent) {

		if (firstEvent->playFrame(ticks)) {

			nextEvent = firstEvent->getNext();
			delete firstEvent;
			firstEvent = nextEvent;

		}

	}


	// Apply as much of that trajectory as possible, without going into the
	// scenery
	localPlayer->move(ticks);


	// Process bullets

	if (firstBullet) {

		if (firstBullet->playFrame(ticks)) {

			nextBullet = firstBullet->getNext();
			delete firstBullet;
			firstBullet = nextBullet;

		}

	}


	// Check for a change in ammo
	if (controls[C_CHANGE].state == SDL_PRESSED) {

		releaseControl(C_CHANGE);

		localPlayer->changeAmmo();

	}

	// Check if time has run out
	if ((ticks > endTime) && localPlayer->getEnergy() && (difficulty >= 2))
		localPlayer->kill(endTime);


	// Calculate viewport
	localPlayer->view(ticks);


	return SUCCESS;

}



void Level::draw (int ticks) {

	GridElement *ge;
	Event *currentEvent;
	Bullet *currentBullet;
	SDL_Rect src, dst;
	int x, y, bgScale;


	// Set tile drawing dimensions
	src.w = TW;
	src.h = TH;
	src.x = 0;


	// Use the player's viewport
	dst.x = 0;
	dst.y = 0;
	dst.w = localPlayer->viewW;
	dst.h = localPlayer->viewH;
	SDL_SetClipRect(screen, &dst);


	// If there is a sky, draw it
	if (bgPE->getType() == PE_SKY) {

		// Background scale
		if (screenW > 320) bgScale = ((screenH - 1) / 100) + 1;
		else bgScale = ((screenH - 34) / 100) + 1;

		dst.x = 0;
		dst.w = screenW;
		dst.h = bgScale;

		for (y = 0; y < localPlayer->viewH; y += bgScale) {

			dst.y = y;
			SDL_FillRect(screen, &dst, 156 + (y / bgScale));

		}

		// Assign the correct portion of the sky palette
		bgPE->setPosition(localPlayer->viewY + (localPlayer->viewH << 9) - F4);

		// Show sun / moon / etc.
		if (skyOrb) {

			dst.x = (localPlayer->viewW * 4) / 5;
			dst.y = (localPlayer->viewH * 3) / 25;
			src.y = skyOrb << 5;
			SDL_BlitSurface(tileSet, &src, screen, &dst);

		}

	} else {

		// If there is no sky, draw a blank background
		// This is only very occasionally actually visible
		SDL_FillRect(screen, NULL, 127);

	}

	// Tell the diagonal lines background where it should be
	if (bgPE->getType() == PE_1D)
		bgPE->setPosition(localPlayer->viewX + localPlayer->viewY);

	// Tell the parallaxing background where it should be
	else if (bgPE->getType() == PE_2D)
		bgPE->setPosition(((localPlayer->viewX >> 10) & 65535) +
			((localPlayer->viewY >> 10) << 16));

	// Tell the underwater darkness effect how dark it should be
	if (bgPE->getType() == PE_WATER)
		bgPE->setPosition(localPlayer->getY() - waterLevel);


	// Show background tiles

	for (y = 0; y <= ((localPlayer->viewH - 1) >> 5) + 1; y++) {

		for (x = 0; x <= ((localPlayer->viewW - 1) >> 5) + 1; x++) {

			// Get the grid element from the given coordinates
			ge = grid[y + (localPlayer->viewY >> 15)] + x +
				(localPlayer->viewX >> 15);

			// If this tile uses a black background, draw it
			if (ge->bg) {

				dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
				dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
				dst.w = dst.h = TW;
				SDL_FillRect(screen, &dst, BLACK);

			}

			// If this is not a foreground tile, draw it
			if ((eventSet[ge->event][E_BEHAVIOUR] != 38) &&
			    ((ge->event < 124) || (ge->event > 125))  ) {

				dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
				dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
				src.y = ge->tile << 5;
				SDL_BlitSurface(tileSet, &src, screen, &dst);

			}

		}

	}


	// Show active events
	currentEvent = firstEvent;

	while (currentEvent) {

		currentEvent->draw(ticks);
		currentEvent = currentEvent->getNext();

	}


	// Show the player

	localPlayer->draw(ticks);


	// Show bullets
	currentBullet = firstBullet;

	while (currentBullet) {

		currentBullet->draw();
		currentBullet = currentBullet->getNext();

	}



	// Show foreground tiles

	for (y = 0; y <= ((localPlayer->viewH - 1) >> 5) + 1; y++) {

		for (x = 0; x <= ((localPlayer->viewW - 1) >> 5) + 1; x++) {

			// Get the grid element from the given coordinates
			ge = grid[y + (localPlayer->viewY >> 15)] + x +
				(localPlayer->viewX >> 15);

			// If this is an "animated" foreground tile, draw it
			if (ge->event == 123) {

				dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
				dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
				if (ticks & 64) src.y = eventSet[ge->event][E_YAXIS] << 5;
				else src.y = eventSet[ge->event][E_MULTIPURPOSE] << 5;
				SDL_BlitSurface(tileSet, &src, screen, &dst);

			}

			// If this is a foreground tile, draw it
			if ((ge->event == 124) || (ge->event == 125) ||
				(eventSet[ge->event][E_BEHAVIOUR] == 38)   ) {

				dst.x = (x << 5) - ((localPlayer->viewX >> 10) & 31);
				dst.y = (y << 5) - ((localPlayer->viewY >> 10) & 31);
				src.y = ge->tile << 5;
				SDL_BlitSurface(tileSet, &src, screen, &dst);

			}

		}

	}

	// Uncomment the following for a line showing the water level
/*	dst.x = 0;
	dst.y = (getWaterLevel(ticks) - localPlayer->viewY) >> 10;
	dst.w = screenW;
	dst.h = 2;
	SDL_FillRect(screen, &dst, 24); */


	SDL_SetClipRect(screen, NULL);


	// The panel
	// Design decision: When the width of the player's viewport is greater than
	// 320, the panel will not fill up the whole space. I decided that as the
	// game used the latin alphabet, and the latin alphabet is read from the
	// left, then the panel should appear to the left. Another option would have
	// been to have it in the centre, but this would obscure Jazz when he was at
	// the bottom of the level. As it is, Jazz is still obscured at viewport
	// widths between 321 and 672. A new approach may be needed, e.g. splitting
	// the panel down the middle.

	// Show panel

	// Change the ammo type display on the panel
	dst.x = 250;
	dst.y = 2;
	SDL_BlitSurface(panelAmmo[localPlayer->getAmmo(false) + 1], NULL, panel,
		&dst);

	dst.x = 0;
	dst.y = screenH - 33;
	SDL_BlitSurface(panel, NULL, screen, &dst);
	dst.y += 32;
	dst.w = 320;
	dst.h = 1;
	SDL_FillRect(screen, &dst, BLACK);


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
		panelSmallFont->showString(":;", 224, screenH - 13);
	else panelSmallFont->showNumber(localPlayer->getAmmo(true), 244,
		screenH - 13);


	// Draw the health bar

	dst.x = 20;
	dst.y = screenH - 13;
	dst.h = 7;
	x = localPlayer->getEnergyBar();

	if (x > F1) {

		dst.w = (x >> 10) - 1;

		x = localPlayer->getEnergy();

		// Choose energy bar colour
		if (x == 4) x = 24;
		else if (x == 3) x = 17;
		else if (x == 2) x = 80;
		else if (x <= 1) x = 32 + (((ticks / 75) * 4) & 15);

		// Draw energy bar
		SDL_FillRect(screen, &dst, x);

		dst.x += dst.w;
		dst.w = 64 - dst.w;

	} else dst.w = 64;


	// Fill in remaining energy bar space with black
	SDL_FillRect(screen, &dst, BLACK);


	return;

}



int Level::run () {

	char *options[5] = {"CONTINUE GAME", "SAVE GAME", "LOAD GAME",
		"SETUP OPTIONS", "QUIT GAME"};
	SDL_Rect dst;
	float smoothfps;
	int stats, paused, menu, option;
	int tickOffset, prevTicks, ticks;
	int count, perfect, timeBonus;


	// Arbitrary initial value
	smoothfps = 50.0f;

	tickOffset = SDL_GetTicks();
	ticks = -10;

	option = menu = paused = 0;

	stats = S_NONE;

	timeBonus = -1;

	while (1) {

		// Do general processing
		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			paused = !paused;
			menu = !menu;
			option = 0;

		}

		if (controls[C_PAUSE].state == SDL_PRESSED) {

			releaseControl(C_PAUSE);

			paused = !paused;

		}

		if (controls[C_STATS].state == SDL_PRESSED) {

			releaseControl(C_STATS);

			stats = (stats + 1) & 3;

		}

		if (menu) {

			// Deal with menu controls

			if (controls[C_UP].state == SDL_PRESSED) {

				releaseControl(C_UP);

				option = (option + 4) % 5;

			}

			if (controls[C_DOWN].state == SDL_PRESSED) {

				releaseControl(C_DOWN);

				option = (option + 1) % 5;

			}

			if (controls[C_ENTER].state == SDL_PRESSED) {

				releaseControl(C_ENTER);

				switch (option) {

					case 0: // Continue

						paused = !paused;
						menu = !menu;

						break;

					case 1: // Save

						break;

					case 2: // Load

						break;

					case 3: // Setup

						// Don't want palette effects in setup menu
						count = bgPE->getType();
						delete firstPE;
						firstPE = NULL;

						if (menuInst->setup() == QUIT) return QUIT;

						// Restore level palette
						usePalette(levelPalette);

						// Restore palette effects
						createPEs(count);

						// Ensure player name string is still valid
						localPlayer->setName(localPlayerName);

						break;

					case 4: // Quit game

						return SUCCESS;

				}

			}

		}


		// Calculate smoothed fps
		smoothfps = smoothfps + 1 - (smoothfps * ((float)mspf) / 1000.0f);
		/* This equation is a simplified version of
		(fps * c) + (smoothfps * (1 - c))
		where c = (1 / fps)
		and fps = 1000 / mspf
		In other words, the response of smoothFPS to changes in FPS
		decreases as the framerate increases 
		The following version is for c = (1 / smoothfps)
		*/
		// smoothfps = (fps / smoothfps) + smoothfps - 1;

		// Ignore outlandish values
		if (smoothfps > 9999) smoothfps = 9999;
		if (smoothfps < 1) smoothfps = 1;


		// Number of ticks of gameplay since the level started

		prevTicks = ticks;
		ticks = SDL_GetTicks() - tickOffset;

		if (paused) {

			tickOffset += ticks - prevTicks;
			ticks = prevTicks;

		} else if (ticks > prevTicks + 100) {

			tickOffset += ticks - (prevTicks + 100);
			ticks = prevTicks + 100;

		}



		// Process frame-by-frame activity

		if (!paused) {

			if (playFrame(ticks) == FAILURE) return FAILURE;


			// Handle player reactions
			switch (localPlayer->reacted(ticks)) {

				case PR_KILLED:

					return LOST;

				case PR_WON:

					return WON;

			}

		}


		// Draw the graphics

		if ((localPlayer->viewW < screenW) || (localPlayer->viewH < screenH))
			SDL_FillRect(screen, NULL, 15);


		draw(ticks);


		if (paused != menu)
			fontmn1->showString("PAUSE", (screenW >> 1) - 44, 32);


		// Draw player list

		if (stats & S_PLAYERS) {

			dst.x = 128;
			dst.y = 11;
			dst.w = 96;

			for (count = 0; count < nPlayers; count++)
				if ((strlen(players[count].getName()) * 8) + 33 > dst.w)
					dst.w = (strlen(players[count].getName()) * 8) + 33;

			dst.h = (nPlayers * 12) + 1;
			SDL_FillRect(screen, &dst, BLACK);

			for (count = 0; count < nPlayers; count++) {

				panelBigFont->showNumber(count + 1, 152, 14 + (count * 12));
				panelBigFont->showString(players[count].getName(), 160,
					14 + (count * 12));

			}

		}


		// Draw graphics statistics

		if (stats & S_SCREEN) {

			dst.x = 236;
			dst.y = 9;
			dst.w = 80;
			dst.h = 32;
			SDL_FillRect(screen, &dst, BLACK);

			panelBigFont->showNumber(screenW, 268, 15);
			panelBigFont->showString("x", 272, 15);
			panelBigFont->showNumber(screenH, 308, 15);
			panelBigFont->showString("fps", 244, 27);
			panelBigFont->showNumber((int)smoothfps, 308, 27);

		}


		// If the level has been won, draw play statistics & bonuses

		if (winTime && (ticks > winTime)) {

			if (timeBonus == -1)
				timeBonus = ((endTime - winTime) / 60000) * 100;
			else {

				count = mspf / 5;
				if (!count) count = 1;

				if (timeBonus - count >= 0) {

					localPlayer->addScore(count);
					timeBonus -= count;

				} else {

					localPlayer->addScore(timeBonus);
					timeBonus = 0;

				}

			}

			fontmn1->showString("TIME", (screenW >> 1) - 152,
				(screenH >> 1) - 60);
			fontmn1->showNumber(timeBonus, (screenW >> 1) + 124,
				(screenH >> 1) - 60);

			fontmn1->showString("ENEMIES", (screenW >> 1) - 152,
				(screenH >> 1) - 40);

			if (enemies)
				fontmn1->showNumber((localPlayer->enemies * 100) / enemies,
					(screenW >> 1) + 124, (screenH >> 1) - 40);
			else
				fontmn1->showNumber(0, (screenW >> 1) + 124,
					(screenH >> 1) - 40);

			fontmn1->showString("ITEMS", (screenW >> 1) - 152,
				(screenH >> 1) - 20);

			if (items)
				fontmn1->showNumber((localPlayer->items * 100) / items,
					(screenW >> 1) + 124, (screenH >> 1) - 20);
			else
				fontmn1->showNumber(0, (screenW >> 1) + 124,
					(screenH >> 1) - 20);

			if ((localPlayer->enemies == enemies) &&
				(localPlayer->items == items)) perfect = 100;
			else perfect = 0;

			fontmn1->showString("PERFECT", (screenW >> 1) - 152, screenH >> 1);
			fontmn1->showNumber(perfect, (screenW >> 1) + 124, screenH >> 1);

			fontmn1->showString("SCORE", (screenW >> 1) - 152,
				(screenH >> 1) + 40);
			fontmn1->showNumber(localPlayer->getScore(), (screenW >> 1) + 124,
				(screenH >> 1) + 40);

		}


		if (menu) {

			// Draw the menu

			dst.x = (screenW >> 2) - 8;
			dst.y = (screenH >> 1) - 46;
			dst.w = 144;
			dst.h = 92;
			SDL_FillRect(screen, &dst, BLACK);

			for (count = 0; count < 5; count++) {

				if (count == option)
					fontmn2->scalePalette(-F2, (-240 * -2) + 48);
				else fontmn2->scalePalette(-F2, (-240 * -2) + 16);

				fontmn2->showString(options[count], screenW >> 2,
					(screenH >> 1) + (count << 4) - 38);

			}

			fontmn2->restorePalette();

		}

	}

	return SUCCESS;

}


void Level::setNext (int newLevelNum, int newWorldNum) {

	char *string;

	string = new char[11];
	sprintf(string, "level%1i.%03i", newLevelNum, newWorldNum);
	menuInst->setNextLevel(string);

	return;

}


GridElement * Level::getGrid (unsigned char gridX, unsigned char gridY) {

	return grid[gridY] + gridX;

}


signed char * Level::getEvent (unsigned char gridX, unsigned char gridY) {

	return eventSet[grid[gridY][gridX].event];

}


signed char * Level::getBullet (unsigned char bullet) {

	return bulletSet[bullet];

}


Sprite * Level::getSprite (unsigned char sprite) {

	return spriteSet + sprite;

}


Anim * Level::getAnim (unsigned char anim) {

	return animSet + anim;

}


Sprite * Level::getFrame (unsigned char anim, unsigned char frame) {

	return animSet[anim].sprites + (frame % animSet[anim].frames);

}


void Level::addTimer () {

	endTime += 2 * 60 * 1000; // 2 minutes. Is this right?

	return;

}


void Level::setWaterLevel (unsigned char gridY) {

	waterLevel = gridY << 15;

}

fixed Level::getWaterLevel (int ticks) {

	if (ticks & 1024) return waterLevel - ((ticks & 1023) * 32);
	return waterLevel - ((1024 - (ticks & 1023)) * 32);

}


void Level::win (int ticks) {

	winTime = ticks;

	return;

}


Scene * Level::createScene () {

	return new Scene(sceneFile);

}


