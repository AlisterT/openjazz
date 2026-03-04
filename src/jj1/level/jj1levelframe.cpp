
/**
 *
 * @file jj1levelframe.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 3rd February 2009: Renamed level.c to level.cpp
 * - 19th July 2009: Created levelframe.cpp from parts of level.cpp
 * - 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
 * - 29th June 2010: Created jj2levelframe.cpp from parts of levelframe.cpp
 * - 1st August 2012: Renamed levelframe.cpp to jj1levelframe.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2013 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Provides the once-per-frame functions for levels.
 *
 */


#include "jj1bullet.h"
#include "event/jj1event.h"
#include "event/jj1guardians.h"
#include "jj1level.h"
#include "jj1levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "setup.h"
#include "util.h"

namespace {
	// Nearest-neighbor scaled blit for 8-bit indexed surfaces (no color key).
	// Scales src to an explicit dstW×dstH rectangle using fixed-point math,
	// supporting non-integer scale ratios (e.g. 320→426 at 3× SDL scale).
	static void blitSurfaceScaled (SDL_Surface *src, SDL_Surface *dst, int dx, int dy, int dstW, int dstH) {
		if (dstW <= 0 || dstH <= 0) return;
		if (SDL_MUSTLOCK(src)) SDL_LockSurface(src);
		if (SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);

		const Uint8 *srcPix = (const Uint8 *)src->pixels;
		Uint8 *dstPix = (Uint8 *)dst->pixels;

		// 16.16 fixed-point ratios
		int xRatio = (src->w << 16) / dstW;
		int yRatio = (src->h << 16) / dstH;

		for (int oy = 0; oy < dstH; oy++) {
			int cy = dy + oy;
			if (cy < 0 || cy >= dst->h) continue;
			int sy = (oy * yRatio) >> 16;
			const Uint8 *srcRow = srcPix + sy * src->pitch;
			Uint8 *dstRow = dstPix + cy * dst->pitch;
			for (int ox = 0; ox < dstW; ox++) {
				int cx = dx + ox;
				if (cx < 0 || cx >= dst->w) continue;
				int sx = (ox * xRatio) >> 16;
				dstRow[cx] = srcRow[sx];
			}
		}

		if (SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);
		if (SDL_MUSTLOCK(src)) SDL_UnlockSurface(src);
	}
}


/**
 * Level iteration.
 *
 * @return Error code
 */
int JJ1Level::step () {

	JJ1Event *event;
	int viewH = canvasH;
	int x, y;

	if(setup.hudStyle == hudType::Classic) {
		// Can we see below the panel?
		if (canvasW > SW) viewH = canvasH;
		else viewH = canvasH - 33;
	} else if(setup.hudStyle == hudType::FPS) {
		// Leave space for panel in view height
		viewH = canvasH - 33;
	}
	// Fixed HUD: reserve space for scaled panel at bottom (integer multiples only)
	if (setup.hudFixed && setup.hudStyle == hudType::Classic && canvasW > SW) {
		int pf_int = canvasW / SW;
		if (pf_int * SW == canvasW)
			viewH = canvasH - 33 * pf_int;
	}

	// Search for active events
	for (y = FTOT(viewY) - 5; y < ITOT(FTOI(viewY) + viewH) + 5; y++) {

		for (x = FTOT(viewX) - 5; x < ITOT(FTOI(viewX) + canvasW) + 5; x++) {

			if ((x >= 0) && (y >= 0) && (x < LW) && (y < LH) &&
				grid[y][x].event && (grid[y][x].event < 121) &&
				(+eventSet[grid[y][x].event].difficulty <= +getDifficulty())) {

				event = events;

				while (event) {

					// If the event has been found, stop searching
					if (event->isFrom(x, y)) break;

					event = event->getNext();

				}

				// If the event wasn't found, create it
				if (!event) {

					switch (getEvent(x, y)->movement) {

						case 28:

							events = new JJ1Bridge(x, y);

							break;

						case 41:

							events = new MedGuardian(x, y);

							break;

						case 60:

							events = new DeckGuardian(x, y);

							break;

						default:

							events = new JJ1StandardEvent(eventSet + grid[y][x].event, x, y, TTOF(x), TTOF(y + 1));

							break;

					}

				}

			}

		}

	}


	// Process bullets
	if (bullets) bullets = bullets->step(ticks);

	// Determine the players' trajectories
	for (x = 0; x < nPlayers; x++) players[x].getJJ1LevelPlayer()->control(ticks);

	// Process active events
	if (events) events = events->step(ticks);

	// Apply as much of those trajectories as possible, without going into the
	// scenery
	for (x = 0; x < nPlayers; x++) players[x].getJJ1LevelPlayer()->move(ticks);


	// Check if time has run out
	if (ticks > endTime) {

		if (multiplayer) {

			game->getMode()->outOfTime();

		} else {

			if ((getDifficulty() == difficultyType::Hard ||
				getDifficulty() == difficultyType::Turbo)
				&& (stage == LS_NORMAL))
				localPlayer->getJJ1LevelPlayer()->kill(nullptr, endTime);

		}

	}


	// Handle change in ammo selection

	x = localPlayer->getAmmoType() + 1;

	if (x != ammoType) {

		// Change the ammo type display on the panel
		ammoType = x;
		ammoOffset = ITOF(26);

	}

	if (ammoOffset > 0) {

		// Descending
		ammoOffset -= F1;

		// Avoid an offset of 0, which prevents changes
		if (ammoOffset == 0) ammoOffset = -1;

	}


	// Handle change in water level
	if (waterLevel < waterLevelTarget) waterLevelSpeed += 3200;
	else waterLevelSpeed -= 3200;
	if (waterLevelSpeed > 80000) waterLevelSpeed = 80000;
	if (waterLevelSpeed < -80000) waterLevelSpeed = -80000;

	waterLevel += waterLevelSpeed >> 6;


	// Handle player reactions
	for (x = 0; x < nPlayers; x++) {

		if (players[x].getJJ1LevelPlayer()->reacted(ticks) == PR_KILLED) {

			players[x].clearAmmo();

			if (!multiplayer) return LOST;

			game->resetPlayer(players + x);

		}

	}


	return E_NONE;

}



/**
 * Draw the level.
 */
void JJ1Level::draw () {

	GridElement *ge;
	SDL_Rect src, dst;
	int viewH = canvasH;
	int vX, vY;
	int x, y;
	unsigned int change;


	// Calculate change since last step
	change = getTimeChange();


	// Calculate viewport
	if (game && (stage == LS_END)) game->view(paused? 0: ((ticks - prevTicks) * 160));
	else localPlayer->getJJ1LevelPlayer()->view(ticks, paused? 0: (ticks - prevTicks), change);

	if(setup.hudStyle == hudType::Classic) {
		// Can we see below the panel?
		if (canvasW > SW) viewH = canvasH;
		else viewH = canvasH - 33;
	} else if(setup.hudStyle == hudType::FPS) {
		// Leave space for panel in view height
		viewH = canvasH - 33;
	}
	// Fixed HUD: reserve space for scaled panel at bottom (integer multiples only)
	if (setup.hudFixed && setup.hudStyle == hudType::Classic && canvasW > SW) {
		int pf_int = canvasW / SW;
		if (pf_int * SW == canvasW)
			viewH = canvasH - 33 * pf_int;
	}

	// Ensure the new viewport is within the level
	if (FTOI(viewX) + canvasW >= TTOI(LW)) viewX = ITOF(TTOI(LW) - canvasW);
	if (viewX < 0) viewX = 0;
	if (FTOI(viewY) + viewH >= TTOI(LH)) viewY = ITOF(TTOI(LH) - viewH);
	if (viewY < 0) viewY = 0;

	// Use the viewport
	dst.x = 0;
	dst.y = 0;
	vX = FTOI(viewX);
	vY = FTOI(viewY);
	dst.w = canvasW;
	dst.h = viewH;
	video.setClipRect(canvas, &dst);


	// Set tile drawing dimensions
	src.w = TTOI(1);
	src.h = TTOI(1);
	src.x = 0;


	// If there is a sky, draw it
	if (sky) {

		// Background scale
		int bgScale;
		if (canvasW > SW) bgScale = ((canvasH - 1) / 100) + 1;
		else bgScale = ((canvasH - 34) / 100) + 1;

		for (y = 0; y < viewH; y += bgScale)
			video.drawRect(0, y, canvasW, bgScale, 156 + (y / bgScale));


		// Show sun / moon / etc.
		if (skyOrb) {

			dst.x = ((canvasW * 4) / 5) - (vX & 3);
			dst.y = ((canvasH - 33) * 3) / 25;
			src.y = TTOI(skyOrb + (vX & 3));
			SDL_BlitSurface(tileSet, &src, canvas, &dst);

		}

	} else {

		// If there is no sky, draw a blank background
		// This is only very occasionally actually visible
		video.clearScreen(127);

	}



	// Show background tiles

	for (y = 0; y <= ITOT(viewH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

			if ((x + ITOT(vX) >= 256) || (y + ITOT(vY) >= 64)) {

				video.drawRect(TTOI(x) - (vX & 31), TTOI(y) - (vY & 31), 32, 32, LEVEL_BLACK);

				continue;

			}

			// Get the grid element from the given coordinates
			ge = grid[y + ITOT(vY)] + x + ITOT(vX);

			// If this tile uses a black background, draw it
			if (ge->bg)
				video.drawRect(TTOI(x) - (vX & 31), TTOI(y) - (vY & 31), 32, 32, LEVEL_BLACK);


			// If this is not a foreground tile, draw it
			if ((ge->event != 124) &&
				(ge->event != 125) &&
				(eventSet[ge->event].movement != 37) &&
				(eventSet[ge->event].movement != 38)) {

				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);
				src.y = TTOI(ge->tile);
				SDL_BlitSurface(tileSet, &src, canvas, &dst);
				// NOTE: hi-res is NOT applied to background tiles here.
				// Background tiles are drawn BEFORE entities on the canvas; queuing hi-res
				// after the canvas would cause them to overdraw player sprites (Z-order bug).

			}

		}

	}


	// Show active events
	if (events) events->draw(ticks, change);


	// Show the players
	for (x = 0; x < nPlayers; x++) players[x].getJJ1LevelPlayer()->draw(ticks, change);


	// Show bullets
	if (bullets) bullets->draw(change);



	// Show foreground tiles
	for (y = 0; y <= ITOT(viewH - 1) + 1; y++) {

		for (x = 0; x <= ITOT(canvasW - 1) + 1; x++) {

			if ((x + ITOT(vX) >= 256) || (y + ITOT(vY) >= 64)) continue;

			// Get the grid element from the given coordinates
			ge = grid[y + ITOT(vY)] + x + ITOT(vX);

			// If this is an "animated" foreground tile, draw it
			if (ge->event == 123) {

				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);
				unsigned char animTile;
				if (ticks & 64) { src.y = TTOI(eventSet[ge->event].multiB); animTile = static_cast<unsigned char>(eventSet[ge->event].multiB); }
				else { src.y = TTOI(eventSet[ge->event].multiA); animTile = static_cast<unsigned char>(eventSet[ge->event].multiA); }
				SDL_BlitSurface(tileSet, &src, canvas, &dst);
				//video.drawRect(dst.x, dst.y, TTOI(1), TTOI(1), 44, false);
			}

			// If this is a foreground tile, draw it
			if ((ge->event == 124) ||
				(ge->event == 125) ||
				(eventSet[ge->event].movement == 37) ||
				(eventSet[ge->event].movement == 38)) {

				dst.x = TTOI(x) - (vX & 31);
				dst.y = TTOI(y) - (vY & 31);
				src.y = TTOI(ge->tile);
				SDL_BlitSurface(tileSet, &src, canvas, &dst);
				//video.drawRect(dst.x, dst.y, TTOI(1), TTOI(1), 33, false);
			}

		}

	}

	// FIXME: Temporary lines showing the water level
	video.drawRect(0, FTOI(waterLevel - viewY), canvasW, 2, 24);
	video.drawRect(0, FTOI(waterLevel - viewY) + 3, canvasW, 1, 24);
	video.drawRect(0, FTOI(waterLevel - viewY) + 6, canvasW, 1, 24);
	video.drawRect(0, FTOI(waterLevel - viewY) + 10, canvasW, 1, 24);

	// Show active guardian's energy bar
	if (events) events->drawEnergy(ticks);


	// If this is a competitive game, draw the score
	if (multiplayer) game->getMode()->drawScore(font);


	// Show panel

	int offsetX = 0;
	bool isWide = (canvasW != SW);

	if(isWide && (setup.hudStyle == hudType::FPS || setup.hudStyle == hudType::Classic)) {
		// center panel in widescreen
		offsetX = (canvasW >> 1) - (SW >> 1);
	}

	// Panel scale for fixed (full-width) HUD mode.
	// Only activates when canvasW is an exact integer multiple of SW so that
	// panel cells and font glyphs scale by the same integer factor.
	float pf = 1.0f;
	int   ps = 1;
	int   panelH = 33;
	if (setup.hudFixed && setup.hudStyle == hudType::Classic && isWide) {
		int pf_int = canvasW / SW; // integer multiples only
		if (pf_int >= 1 && pf_int * SW == canvasW) {
			pf      = (float)pf_int;
			panelH  = 33 * pf_int;
			ps      = pf_int;
			offsetX = 0; // panel spans full width
		}
		// else: non-integer ratio — fall back to floating (offsetX unchanged)
	}
	// Helper: scale a horizontal panel-space coordinate to canvas pixels
	auto S  = [pf](int v) { return (int)(v * pf + 0.5f); };
	// Helper: map "v pixels from canvas bottom" to the correct canvas y in the
	// NN-scaled panel. Source row (33-v) first appears at dest row
	// ceil((33-v)*panelH/33), so canvas y = panelTop + ceil((33-v)*panelH/33).
	auto Sy = [panelH](int v) -> int {
		return (canvasH - panelH) + (int)ceilf((33 - v) * panelH / 33.0f);
	};

	video.setClipRect(canvas, nullptr);

	if (ammoOffset != 0) {

		if (ammoOffset < 0) {

			// Finished descending
			ammoOffset = 0;

		}

		src.x = 0;
		src.y = FTOI(ammoOffset);
		src.w = 64;
		src.h = 26 - src.y;
		dst.x = 248;
		dst.y = 3;
		SDL_BlitSurface(panelAmmo[ammoType], &src, panel, &dst);

	}

	// always classic HUD if there is not enough space
	if(setup.hudStyle == hudType::Classic ||
		(setup.hudStyle == hudType::FPS && !isWide)) {

		if (pf > 1.0f) {
			// Fixed HUD at integer scale: panel fills full canvas width
			blitSurfaceScaled(panel, canvas, 0, canvasH - panelH, canvasW, panelH);
			video.drawRect(0, canvasH - 1, canvasW, 1, LEVEL_BLACK);
		} else {
			dst.x = offsetX;
			dst.y = canvasH - 33;
			SDL_BlitSurface(panel, nullptr, canvas, &dst);
			// Fill the one missing pixel row at the bottom black
			video.drawRect(offsetX, canvasH - 1, SW, 1, LEVEL_BLACK);
		}
	} else if (setup.hudStyle == hudType::FPS) {
		// quake-style HUD

		dst.y = canvasH - 33;

		for (x = 0; x <= ITOT(offsetX); x++) {
			// draw left border
			dst.x = TTOI(x);
			SDL_BlitSurface(panelBG[0], nullptr, canvas, &dst);

			// draw right border
			dst.x = canvasW - TTOI(x + 1);
			SDL_BlitSurface(panelBG[1], nullptr, canvas, &dst);
		}

		// cut the panel borders and re-center
		src.x = 1; // left: 1 white pixel
		src.y = 0;
		src.w = SW - 8; // right: 3 black pixels + 5 pixels around screws
		src.h = TTOI(1);
		dst.x = offsetX + 5;
		SDL_BlitSurface(panel, &src, canvas, &dst);

		// Fill the one missing pixel row at the bottom black
		video.drawRect(0, canvasH - 1, canvasW, 1, LEVEL_BLACK);

		offsetX += 4; // move everything inside right
	}


	// Show panel data

	// Show score
	panelSmallFont->showNumber(localPlayer->getScore(), offsetX + S(84), Sy(27), ps);

	// Show time remaining
	if (endTime > ticks) x = endTime - ticks;
	else x = 0;
	y = x / (60 * 1000);
	panelSmallFont->showNumber(y, offsetX + S(116), Sy(27), ps);
	x -= (y * 60 * 1000);
	y = x / 1000;
	panelSmallFont->showNumber(y, offsetX + S(136), Sy(27), ps);
	x -= (y * 1000);
	y = x / 100;
	panelSmallFont->showNumber(y, offsetX + S(148), Sy(27), ps);

	// Show lives
	panelSmallFont->showNumber(localPlayer->getLives(), offsetX + S(124), Sy(13), ps);

	// Show planet number

	if (worldNum <= 41) // Main game levels
		panelSmallFont->showNumber((worldNum % 3) + 1, offsetX + S(184), Sy(13), ps);
	else if ((worldNum >= 50) && (worldNum <= 52)) // Christmas levels
		panelSmallFont->showNumber(worldNum - 49, offsetX + S(184), Sy(13), ps);
	else panelSmallFont->showNumber(worldNum, offsetX + S(184), Sy(13), ps);

	// Show level number
	panelSmallFont->showNumber(levelNum + 1, offsetX + S(196), Sy(13), ps);

	// Show ammo
	if (localPlayer->getAmmoType() == -1) {

		// Draw "infinity" symbol
		panelSmallFont->showString(":", offsetX + S(225), Sy(13),
			alignX::Left, alignY::Top, ps);
		panelSmallFont->showString(";", offsetX + S(233), Sy(13),
			alignX::Left, alignY::Top, ps);

	} else {

		x = localPlayer->getAmmo();

		// Trailing 0s
		if (x < 100) {

			panelSmallFont->showNumber(0, offsetX + S(229), Sy(13), ps);
			if (x < 10) panelSmallFont->showNumber(0, offsetX + S(237), Sy(13), ps);

		}

		panelSmallFont->showNumber(x > 999? 999: x, offsetX + S(245), Sy(13), ps);

	}


	// Draw the health bar

	x = localPlayer->getJJ1LevelPlayer()->getEnergy();
	y = (ticks - prevTicks) * 40;

	if (FTOI(energyBar) < x) {
		// increase

		if (ITOF(x) - energyBar < y) energyBar = ITOF(x);
		else energyBar += y;

	} else if (FTOI(energyBar) > x) {
		// decrease

		if (energyBar - ITOF(x) < y) energyBar = ITOF(x);
		else energyBar -= y;

	}

	int barFilled = 0;
	if (energyBar > F1) {

		barFilled = (int)((FTOI(energyBar) - 1) * pf + 0.5f);

		// Choose energy bar colour
		int color;
		if (x <= 20) color = 32 + (((ticks / 75) * 4) & 15); // flash
		else if (x > 51) color = 24; // blue only before first hit
		else if (x > 38) color = 17; // green
		else if (x > 25) color = 80; // pink
		else color = 32; // orange is only seen in easy

		// Draw energy bar
		video.drawRect(offsetX + S(20), Sy(13), barFilled, S(7), color);

	}

	// Fill in remaining energy bar space with black
	video.drawRect(offsetX + S(20) + barFilled, Sy(13), S(64) - barFilled, S(7), LEVEL_BLACK);
}
