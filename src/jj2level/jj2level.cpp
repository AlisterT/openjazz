
/**
 *
 * @file jj2level.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 22nd July 2008: Created levelload.c from parts of level.c
 * 3rd February 2009: Renamed level.c to level.cpp
 * 19th July 2009: Added parts of levelload.cpp to level.cpp
 * 29th June 2010: Created jj2level.cpp from parts of level.cpp
 * 2nd July 2010: Created jj2event.cpp from parts of jj2level.cpp
 * 2nd July 2010: Created jj2eventframe.cpp from parts of jj2level.cpp
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
 * Deals with the creating, playing and freeing of JJ2 levels.
 *
 */


#include "jj2event/jj2event.h"
#include "jj2level.h"
#include "jj2levelplayer/jj2levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "util.h"

#include <string.h>


/**
 * Create a JJ2 level.
 *
 * @param owner The current game
 * @param fileName Name of the file containing the level data.
 * @param checkpoint Whether or not the player(s) will start at a checkpoint
 * @param multi Whether or not the level will be multi-player
 */
JJ2Level::JJ2Level (Game* owner, char* fileName, bool checkpoint, bool multi) :
	Level(owner) {

	int ret;

	// Load level data

	ret = load(fileName, checkpoint);

	if (ret < 0) throw ret;

	multiplayer = multi;

	return;

}


/**
 * Delete the JJ2 level.
 */
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


/**
 * Determine whether or not the given point is solid when travelling upwards.
 *
 * @param x X-coordinate
 * @param y Y-coordinate
 *
 * @return Solidity
 */
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


/**
 * Determine whether or not the given point is solid when travelling downwards.
 *
 * @param x X-coordinate
 * @param y Y-coordinate
 * @param drop Whether or not the player is dropping
 *
 * @return Solidity
 */
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


/**
 * Set which level will come next.
 *
 * @param fileName Next level's file name
 */
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


/**
 * Set the frame of the animated tile at the given location.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param frame The new frame
 */
void JJ2Level::setFrame (int gridX, int gridY, unsigned char frame) {

	unsigned char buffer[MTL_L_GRID];

	layer->setFrame(gridX, gridY, frame);

	if (multiplayer) {

		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX & 0xFF;
		buffer[3] = gridY & 0xFF;
		buffer[4] = 0; // tile variable
		buffer[5] = frame;
		buffer[6] = (gridX >> 8) & 0xFF;
		buffer[7] = (gridY >> 8) & 0xFF;

		game->send(buffer);

	}

	return;

}


/**
 * Get the modifier event for the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 *
 * @return Modifier event
 */
JJ2Modifier* JJ2Level::getModifier (int gridX, int gridY) {

	return mods[gridY] + gridX;

}


/**
 * Get a sprite.
 *
 * @param sprite Sprite number
 *
 * @return Sprite
 */
Sprite* JJ2Level::getSprite (unsigned char sprite) {

	return spriteSet + sprite;

}


/**
 * Get an animation.
 *
 * @param set Animation set number
 * @param anim Animation number
 * @param flipped Whether or not the animation should be flipped horizontally
 *
 * @return Animation
 */
Anim* JJ2Level::getAnim (int set, int anim, bool flipped) {

	return (flipped? flippedAnimSets: animSets)[set] + anim;

}


/**
 * Get a player animation.
 *
 * @param character Character
 * @param anim Animation number
 * @param flipped Whether or not the animation should be flipped horizontally
 *
 * @return Animation
 */
Anim* JJ2Level::getPlayerAnim (int character, int anim, bool flipped) {

	int set;

	if (TSF) {

		if (character == 1) set = 61;
		else if (character == 2) set = 89;
		else set = 55;

	} else {

		if (character == 1) set = 85;
		else set = 54;

	}

	return getAnim(set, playerAnims[anim], flipped);

}


/**
 * Set the water level.
 *
 * @param gridY New water level y-coordinate
 * @param instant Whether or not the change is instant
 */
void JJ2Level::setWaterLevel (int gridY, bool instant) {

	unsigned char buffer[MTL_L_PROP];

	waterLevelTarget = TTOF(gridY);

	if (instant) waterLevel = waterLevelTarget - F8;

	if (multiplayer) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 1; // set water level
		buffer[3] = gridY & 0xFF;
		buffer[4] = (gridY >> 8) & 0xFF;

		game->send(buffer);

	}

	return;

}


/**
 * Determine the water level.
 *
 * @return The y-coordinate of the water level
 */
fixed JJ2Level::getWaterLevel () {

	return waterLevel;

}


/**
 * Move a player to a warp target.
 *
 * @param player The player to move
 * @param id The warp target ID
 */
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


/**
 * Interpret data received from client/server
 *
 * @param buffer Received data
 */
void JJ2Level::receive (unsigned char* buffer) {

	switch (buffer[1]) {

		case MT_L_PROP:

			if (buffer[2] == 1) {

				waterLevelTarget = TTOF(buffer[3] + (buffer[4] << 8));

			} else if (buffer[2] == 2) {

				if (stage == LS_NORMAL)
					endTime += buffer[3] * 1000;

			}

			break;

		case MT_L_GRID:

			if (buffer[4] == 0) layer->setFrame(buffer[2] + (buffer[6] << 8), buffer[3] + (buffer[7] << 8), buffer[5]);

			break;

		case MT_L_STAGE:

			stage = LevelStage(buffer[2]);

			break;

	}

	return;

}


/**
 * Play the level.
 *
 * @return Error code
 */
int JJ2Level::play () {

	JJ2LevelPlayer* jj2LevelPlayer;
	bool pmessage, pmenu;
	int option;
	unsigned int returnTime;
	int count, ret;


	jj2LevelPlayer = localPlayer->getJJ2LevelPlayer();

	tickOffset = globalTicks;
	ticks = T_STEP;
	steps = 0;

	pmessage = pmenu = false;
	option = 0;

	returnTime = 0;

	video.setPalette(palette);

	playMusic(musicFile);

	while (true) {

		ret = loop(pmenu, option, pmessage);

		if (ret < 0) return ret;


		// Check if level has been won
		if (game && returnTime && (ticks > returnTime)) {

			ret = game->setLevel(nextLevel);

			if (ret < 0) return ret;

			return WON;

		}


		// Process frame-by-frame activity

		while (getTimeChange() >= T_STEP) {

			// Apply controls to local player
			for (count = 0; count < PCONTROLS; count++)
				localPlayer->setControl(count, controls.getState(count));

			ret = step();
			steps++;

			if (ret) return ret;

		}


		// Draw the graphics

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			font->showString("pause", (canvasW >> 1) - 44, 32);


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


		// Draw statistics, menu etc.
		drawOverlay(JJ2_BLACK, pmenu, option, 71, 31, -8);

	}

	return E_NONE;

}


