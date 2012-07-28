
/**
 *
 * @file level.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c
 * 1st January 2006: Created events.c from parts of level.c
 * 22nd July 2008: Created levelload.c from parts of level.c
 * 3rd February 2009: Renamed level.c to level.cpp
 * 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 18th July 2009: Created demolevel.cpp from parts of level.cpp and
 *                 levelload.cpp
 * 19th July 2009: Created levelframe.cpp from parts of level.cpp
 * 19th July 2009: Added parts of levelload.cpp to level.cpp
 * 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
 * 29th June 2010: Created jj2level.cpp from parts of level.cpp
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
 * Deals with the creating, playing and freeing of levels.
 *
 */


#include "bullet.h"
#include "event/event.h"
#include "level.h"

#include "bonus/bonus.h"
#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "player/levelplayer.h"
#include "scene/scene.h"
#include "util.h"

#include <string.h>


/**
 * Base constructor for DemoLevel sub-class.
 */
Level::Level (Game* owner) : BaseLevel(owner) {

	// Do nothing

	return;

}


/**
 * Create a JJ1 level.
 *
 * @param fileName Name of the file containing the level data.
 * @param diff Difficulty level
 * @param checkpoint Whether or not the player(s) will start at a checkpoint
 * @param multi Whether or not the level will be multi-player
 */
Level::Level (Game* owner, char* fileName, bool checkpoint, bool multi) :
	BaseLevel (owner) {

	int ret;

	// Load level data

	ret = load(fileName, checkpoint);

	if (ret < 0) throw ret;

	multiplayer = multi;

	return;

}


/**
 * Delete HUD graphical data.
 */
void Level::deletePanel () {

	SDL_FreeSurface(panel);
	SDL_FreeSurface(panelAmmo[0]);
	SDL_FreeSurface(panelAmmo[1]);
	SDL_FreeSurface(panelAmmo[2]);
	SDL_FreeSurface(panelAmmo[3]);
	SDL_FreeSurface(panelAmmo[4]);

	return;

}


/**
 * Delete the JJ1 level.
 */
Level::~Level () {

	int count;

	// Free events
	if (events) delete events;

	// Free bullets
	if (bullets) delete bullets;

	for (count = 0; count < PATHS; count++) {

		delete[] path[count].x;
		delete[] path[count].y;

	}

	delete[] sceneFile;
	delete[] musicFile;

	delete[] spriteSet;

	SDL_FreeSurface(tileSet);

	deletePanel();

	delete font;

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
bool Level::checkMaskUp (fixed x, fixed y) {

	GridElement *ge;

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x > TTOF(LW)) || (y > TTOF(LH)))
		return true;

	ge = grid[FTOT(y)] + FTOT(x);

	// Event 122 is one-way
	if (ge->event == 122) return false;

	// Check the mask in the tile in question
	return mask[ge->tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


/**
 * Determine whether or not the given point is solid when travelling downwards.
 *
 * @param x X-coordinate
 * @param y Y-coordinate
 *
 * @return Solidity
 */
bool Level::checkMaskDown (fixed x, fixed y) {

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x > TTOF(LW)) || (y > TTOF(LH)))
		return true;

	// Check the mask in the tile in question
	return mask[grid[FTOT(y)][FTOT(x)].tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


/**
 * Determine whether or not the given point should cause damage to the player.
 *
 * @param x X-coordinate
 * @param y Y-coordinate
 *
 * @return Painful solidity
 */
bool Level::checkSpikes (fixed x, fixed y) {

	GridElement *ge;

	// Anything off the edge of the map is not spikes
	// Ignore the bottom, as it is deadly anyway
	if ((x < 0) || (y < 0) || (x > TTOF(LW))) return false;

	ge = grid[FTOT(y)] + FTOT(x);

	// Event 126 is spikes
	if (ge->event != 126) return false;

	// Check the mask in the tile in question
	return mask[ge->tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


/**
 * Determine the level's world number.
 *
 * @return World number
 */
int Level::getWorld() {

	return worldNum;

}


/**
 * Set which level will come next.
 *
 * @param nextLevel Next level's number
 * @param nextWorld Next level's world number
 */
void Level::setNext (int nextLevel, int nextWorld) {

	unsigned char buffer[MTL_L_PROP];

	nextLevelNum = nextLevel;
	nextWorldNum = nextWorld;

	if (multiplayer) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 0; // set next level
		buffer[3] = nextLevel;
		buffer[4] = nextWorld;

		game->send(buffer);

	}

	return;

}


/**
 * Set the tile at the given location.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param tile The new tile
 */
void Level::setTile (unsigned char gridX, unsigned char gridY, unsigned char tile) {

	unsigned char buffer[MTL_L_GRID];

	grid[gridY][gridX].tile = tile;

	if (multiplayer) {

		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 0; // tile variable
		buffer[5] = tile;

		game->send(buffer);

	}

	return;

}


/**
 * Get the active events.
 *
 * @return The first active event
 */
Event* Level::getEvents () {

	return events;

}


/**
 * Get the event data for the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 *
 * @return Event data
 */
EventType* Level::getEvent (unsigned char gridX, unsigned char gridY) {

	int event = grid[gridY][gridX].event;

	if (event) return eventSet + event;

	return NULL;

}


/**
 * Get the hits incurred by the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 *
 * @return Number of hits
 */
unsigned char Level::getEventHits (unsigned char gridX, unsigned char gridY) {

	return grid[gridY][gridX].hits;

}


/**
 * Get the set time for the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 *
 * @return Time
 */
unsigned int Level::getEventTime (unsigned char gridX, unsigned char gridY) {

	return grid[gridY][gridX].time;

}


/**
 * Remove the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 */
void Level::clearEvent (unsigned char gridX, unsigned char gridY) {

	unsigned char buffer[MTL_L_GRID];

	// Ignore if the event has been un-destroyed
	if (!grid[gridY][gridX].hits &&
		eventSet[grid[gridY][gridX].event].strength) return;

	grid[gridY][gridX].event = 0;

	if (multiplayer) {

		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 2; // event variable
		buffer[5] = 0;

		game->send(buffer);

	}

	return;

}


/**
 * Register a hit on the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param source The player that fired the bullet that hit the event
 *
 * @return The remaining number of hits until the event is destroyed
 */
int Level::hitEvent (unsigned char gridX, unsigned char gridY, LevelPlayer* source) {

	GridElement* ge;
	unsigned char buffer[MTL_L_GRID];
	int hitsToKill;

	ge = grid[gridY] + gridX;

	hitsToKill = eventSet[ge->event].strength;

	// If the event cannot be hit, return negative
	if (!hitsToKill) return -1;

	// Increase the hit count
	ge->hits++;

	// Check if the event has been killed
	if (ge->hits == hitsToKill) {

		// Notify the player that shot the bullet
		// If this returns false, ignore the hit
		if (!source->takeEvent(gridX, gridY, ticks)) {

			ge->hits--;

			return 1;

		}

	}

	if (multiplayer) {

		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 3; // hits variable
		buffer[5] = ge->hits;

		game->send(buffer);

	}

	return hitsToKill - ge->hits;

}


/**
 * Set the time of the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param time Time
 */
void Level::setEventTime (unsigned char gridX, unsigned char gridY, unsigned int time) {

	grid[gridY][gridX].time = time;

	return;

}


/**
 * Get the bullet data for the given bullet type.
 *
 * @param bullet Bullet type
 *
 * @return Bullet data
 */
signed char* Level::getBullet (unsigned char bullet) {

	return bulletSet[bullet];

}


/**
 * Get a sprite.
 *
 * @param sprite Sprite number
 *
 * @return Sprite
 */
Sprite* Level::getSprite (unsigned char sprite) {

	return spriteSet + sprite;

}


/**
 * Get an animation.
 *
 * @param anim Animation number
 *
 * @return Animation
 */
Anim* Level::getAnim (unsigned char anim) {

	return animSet + anim;

}


/**
 * Get a "miscellaneous" animation.
 *
 * @param anim Animation number
 *
 * @return Animation
 */
Anim* Level::getMiscAnim (unsigned char anim) {

	return animSet + miscAnims[anim];

}


/**
 * Set the water level.
 *
 * @param gridY New water level y-coordinate
 */
void Level::setWaterLevel (unsigned char gridY) {

	unsigned char buffer[MTL_L_PROP];

	waterLevelTarget = TTOF(gridY);

	if (multiplayer) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 1; // set water level
		buffer[3] = gridY;
		buffer[4] = 0; // Doesn't really matter

		game->send(buffer);

	}

	return;

}


/**
 * Determine the water level.
 *
 * @return The y-coordinate of the water level
 */
fixed Level::getWaterLevel () {

	return waterLevel;

}


/**
 * Play a sound.
 *
 * @param sound Number of the sound to play
 */
void Level::playSound (int sound) {

	if (sound > 0) ::playSound(soundMap[sound - 1]);

	return;

}


/**
 * Start a flash palette effect.
 *
 * @param red Red component of flash colour
 * @param green Green component of flash colour
 * @param blue Blue component of flash colour
 * @param duration Duration of the flash effect
 */
void Level::flash (unsigned char red, unsigned char green, unsigned char blue, int duration) {

	paletteEffects = new FlashPaletteEffect(red, green, blue, duration, paletteEffects);

	return;

}


/**
 * Play the bonus level.
 *
 * @return Error code
 */
int Level::playBonus () {

	Bonus *bonus;
	char *bonusFile;
	int ret;

	if (!localPlayer->getLevelPlayer()->hasGem()) return E_NONE;

	delete paletteEffects;
	paletteEffects = NULL;

	bonusFile = createFileName(F_BONUSMAP, 0);

	try {

		baseLevel = bonus = new Bonus(game, bonusFile, multiplayer);

	} catch (int e) {

		return e;

	}

	delete[] bonusFile;

	ret = bonus->play();

	delete bonus;
	baseLevel = NULL;

	if (ret == E_NONE) playMusic("menusng.psm");

	return ret;

}


/**
 * Interpret data received from client/server
 *
 * @param buffer Received data
 */
void Level::receive (unsigned char* buffer) {

	switch (buffer[1]) {

		case MT_L_PROP:

			if (buffer[2] == 0) {

				nextLevelNum = buffer[3];
				nextWorldNum = buffer[4];

			} else if (buffer[2] == 1) {

				waterLevelTarget = TTOF(buffer[3]);

			} else if (buffer[2] == 2) {

				if (stage == LS_NORMAL)
					endTime += buffer[3] * 1000;

			}

			break;

		case MT_L_GRID:

			if (buffer[4] == 0) grid[buffer[3]][buffer[2]].tile = buffer[5];
			else if (buffer[4] == 2)
				grid[buffer[3]][buffer[2]].event = buffer[5];
			else if (buffer[4] == 3)
				grid[buffer[3]][buffer[2]].hits = buffer[5];

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
int Level::play () {

	LevelPlayer* levelPlayer;
	char *string;
	bool pmessage, pmenu;
	int option;
	unsigned int returnTime;
 	int perfect;
 	int timeBonus;
 	int count;


	levelPlayer = localPlayer->getLevelPlayer();

	tickOffset = globalTicks;
	ticks = 17;
	steps = 0;

	pmessage = pmenu = false;
	option = 0;

	returnTime = 0;
	timeBonus = -1;
	perfect = 0;

	video.setPalette(palette);

	playMusic(musicFile);

	while (true) {

		count = loop(pmenu, option, pmessage);

		if (count <= 0) return count;


		// Check if level has been won
		if (game && returnTime && (ticks > returnTime)) {

			if (!multiplayer) {

				// If the gem has been collected, play the bonus level
				if (playBonus() == E_QUIT) return E_QUIT;

			}

			if (nextLevelNum == 99) {

				if (playScene(sceneFile) == E_QUIT) return E_QUIT;

				count = game->setLevel(NULL);

			} else {

				string = createFileName(F_LEVEL, nextLevelNum, nextWorldNum);
				count = game->setLevel(string);
				delete[] string;

			}

			if (count < 0) return count;

			return WON;

		}


		// Process frame-by-frame activity

		while (getTimeChange() >= 17) {

			bool playerWasAlive = (localPlayer->getLevelPlayer()->getEnergy() != 0);

			// Apply controls to local player
			for (count = 0; count < PCONTROLS; count++)
				localPlayer->setControl(count, controls.getState(count));

			count = step();
			steps++;

			if (count) return count;

			if (!multiplayer && playerWasAlive && (localPlayer->getLevelPlayer()->getEnergy() == 0))
				flash(0, 0, 0, T_END << 1);

		}


		// Draw the graphics

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			font->showString("pause", (canvasW >> 1) - 44, 32);


		if (stage == LS_END) {

			// The level is over, so draw play statistics & bonuses

			// Apply time bonus

			if (timeBonus) {

				count = (ticks - prevTicks) / 100;
				if (!count) count = 1;

				if (timeBonus == -1) {

					if (ticks < endTime) timeBonus = ((endTime - ticks) / 60000) * 100;
					else timeBonus = 0;

					if ((levelPlayer->getEnemies() == enemies) &&
						(levelPlayer->getItems() == items)) perfect = 100;

				} else if (timeBonus - count >= 0) {

					localPlayer->addScore(count * 10);
					timeBonus -= count;

				} else {

					localPlayer->addScore(timeBonus * 10);
					timeBonus = 0;

				}

				if (timeBonus == 0) {

					returnTime = ticks + T_END;
					paletteEffects = new WhiteOutPaletteEffect(T_END, paletteEffects);
					::playSound(S_UPLOOP);

				}

			}

			// Display statistics & bonuses

			font->showString("time", (canvasW >> 1) - 152, (canvasH >> 1) - 60);
			font->showNumber(timeBonus, (canvasW >> 1) + 124, (canvasH >> 1) - 60);

			font->showString("enemies", (canvasW >> 1) - 152, (canvasH >> 1) - 40);

			if (enemies)
				font->showNumber((levelPlayer->getEnemies() * 100) / enemies, (canvasW >> 1) + 124, (canvasH >> 1) - 40);
			else
				font->showNumber(0, (canvasW >> 1) + 124, (canvasH >> 1) - 40);
			font->showString("%", (canvasW >> 1) + 124, (canvasH >> 1) - 40);

			font->showString("items", (canvasW >> 1) - 152, (canvasH >> 1) - 20);

			if (items)
				font->showNumber((levelPlayer->getItems() * 100) / items, (canvasW >> 1) + 124, (canvasH >> 1) - 20);
			else
				font->showNumber(0, (canvasW >> 1) + 124, (canvasH >> 1) - 20);
			font->showString("%", (canvasW >> 1) + 124, (canvasH >> 1) - 20);

			font->showString("perfect", (canvasW >> 1) - 152, canvasH >> 1);
			font->showNumber(perfect, (canvasW >> 1) + 124, canvasH >> 1);

			font->showString("score", (canvasW >> 1) - 152, (canvasH >> 1) + 40);
			font->showNumber(localPlayer->getScore(), (canvasW >> 1) + 124, (canvasH >> 1) + 40);

		}


		// Draw statistics, menu etc.
		drawOverlay(LEVEL_BLACK, pmenu, option, 15, 47, -16);

	}

	return E_NONE;

}


