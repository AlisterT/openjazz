
/**
 *
 * @file jj1level.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 1st January 2006: Created events.c from parts of level.c
 * - 22nd July 2008: Created levelload.c from parts of level.c
 * - 3rd February 2009: Renamed level.c to level.cpp
 * - 5th February 2009: Added parts of events.cpp and level.cpp to player.cpp
 * - 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * - 18th July 2009: Created demolevel.cpp from parts of level.cpp and
 *                 levelload.cpp
 * - 19th July 2009: Created levelframe.cpp from parts of level.cpp
 * - 19th July 2009: Added parts of levelload.cpp to level.cpp
 * - 30th March 2010: Created baselevel.cpp from parts of level.cpp and
 *                  levelframe.cpp
 * - 29th June 2010: Created jj2level.cpp from parts of level.cpp
 * - 1st August 2012: Renamed level.cpp to jj1level.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the creating, playing and freeing of levels.
 *
 */


#include "jj1bullet.h"
#include "event/jj1event.h"
#include "jj1level.h"
#include "jj1levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/paletteeffects.h"
#include "io/gfx/sprite.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "io/log.h"
#include "util.h"

#include <string.h>


/**
 * Base constructor for JJ1DemoLevel sub-class.
 *
 * @param owner The current game
 */
JJ1Level::JJ1Level (Game* owner) : Level(owner) {
	tileSet = panel = NULL;
	events = NULL;
	bullets = NULL;
	sceneFile = NULL;
	spriteSet = NULL;
	sky = false;
	skyOrb = 0;
	levelNum = worldNum = nextLevelNum = nextWorldNum = 0;
	enemies = 0;
	waterLevel = waterLevelTarget = waterLevelSpeed = 0;
	energyBar = ammoType = ammoOffset = 0;
	font = NULL;
	musicFile = NULL;
}


/**
 * Create a JJ1 level.
 *
 * @param owner The current game
 * @param fileName Name of the file containing the level data.
 * @param checkpoint Whether or not the player(s) will start at a checkpoint
 * @param multi Whether or not the level will be multi-player
 */
JJ1Level::JJ1Level (Game* owner, char* fileName, bool checkpoint, bool multi) :
	Level (owner) {

	// Load level data
	int ret = load(fileName, checkpoint);
	if (ret < 0) throw ret;

	multiplayer = multi;

}


/**
 * Delete HUD graphical data.
 */
void JJ1Level::deletePanel () {

	SDL_FreeSurface(panel);
	for (int i = 0; i < 6; i++)
		SDL_FreeSurface(panelAmmo[i]);

}


/**
 * Delete the JJ1 level.
 */
JJ1Level::~JJ1Level () {

	// Free events
	if (events) delete events;

	// Free bullets
	if (bullets) delete bullets;

	for (int i = 0; i < PATHS; i++) {

		delete[] path[i].x;
		delete[] path[i].y;

	}

	delete[] sceneFile;
	delete[] musicFile;

	delete[] spriteSet;

	SDL_FreeSurface(tileSet);

	deletePanel();

	delete font;

	resampleSounds();

	video.setTitle(NULL);

}


/**
 * Determine whether or not the given point is solid when travelling upwards.
 *
 * @param x X-coordinate
 * @param y Y-coordinate
 *
 * @return Solidity
 */
bool JJ1Level::checkMaskUp (fixed x, fixed y) {

	GridElement *ge;

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x >= TTOF(LW)) || (y >= TTOF(LH)))
		return true;

	ge = grid[FTOT(y)] + FTOT(x);

	// JJ1Event 122 is one-way
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
bool JJ1Level::checkMaskDown (fixed x, fixed y) {

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x >= TTOF(LW)) || (y >= TTOF(LH)))
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
bool JJ1Level::checkSpikes (fixed x, fixed y) {

	GridElement *ge;

	// Anything off the edge of the map is not spikes
	// Ignore the bottom, as it is deadly anyway
	if ((x < 0) || (y < 0) || (x > TTOF(LW))) return false;

	ge = grid[FTOT(y)] + FTOT(x);

	// JJ1Event 126 is spikes
	if (ge->event != 126) return false;

	// Check the mask in the tile in question
	return mask[ge->tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


/**
 * Determine the level's world number.
 *
 * @return World number
 */
int JJ1Level::getWorld() {

	return worldNum;

}


/**
 * Set which level will come next.
 *
 * @param nextLevel Next level's number
 * @param nextWorld Next level's world number
 */
void JJ1Level::setNext (int nextLevel, int nextWorld) {

	nextLevelNum = nextLevel;
	nextWorldNum = nextWorld;

	if(nextLevelNum == 99 || nextWorldNum == 99) {
		LOG_MAX("No next level available, episode/game will end");
	} else {
		LOG_MAX("Next level is LEVEL%d.%03d", nextLevelNum, nextWorldNum);
	}

	if (multiplayer) {

		unsigned char buffer[MTL_L_PROP];
		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 0; // set next level
		buffer[3] = nextLevel;
		buffer[4] = nextWorld;

		game->send(buffer);

	}

}


/**
 * Set the tile at the given location.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param tile The new tile
 */
void JJ1Level::setTile (unsigned char gridX, unsigned char gridY, unsigned char tile) {

	grid[gridY][gridX].tile = tile;

	if (multiplayer) {

		unsigned char buffer[MTL_L_GRID];
		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 0; // tile variable
		buffer[5] = tile;

		game->send(buffer);

	}

}


/**
 * Get the active events.
 *
 * @return The first active event
 */
JJ1Event* JJ1Level::getEvents () {

	return events;

}


/**
 * Get the event data for the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 *
 * @return JJ1Event data
 */
JJ1EventType* JJ1Level::getEvent (unsigned char gridX, unsigned char gridY) {

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
unsigned char JJ1Level::getEventHits (unsigned char gridX, unsigned char gridY) {

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
unsigned int JJ1Level::getEventTime (unsigned char gridX, unsigned char gridY) {

	return grid[gridY][gridX].time;

}


/**
 * Remove the event from the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 */
void JJ1Level::clearEvent (unsigned char gridX, unsigned char gridY) {

	// Ignore if the event has been un-destroyed
	if (!grid[gridY][gridX].hits &&
		eventSet[grid[gridY][gridX].event].strength) return;

	grid[gridY][gridX].event = 0;

	if (multiplayer) {

		unsigned char buffer[MTL_L_GRID];
		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 2; // event variable
		buffer[5] = 0;

		game->send(buffer);

	}

}


/**
 * Register hit(s) on the event for the given tile.
 *
 * @param gridX X-coordinate of the tile
 * @param gridY Y-coordinate of the tile
 * @param hits The number of hits to attempt to inflict on the event
 * @param source The player that inflicted the hit(s)
 * @param time Time
 *
 * @return The remaining number of hits until the event is destroyed
 */
int JJ1Level::hitEvent (unsigned char gridX, unsigned char gridY, int hits, JJ1LevelPlayer* source, unsigned int time) {

	GridElement* ge;
	int hitsToKill;

	ge = grid[gridY] + gridX;

	hitsToKill = eventSet[ge->event].strength;

	// If the event cannot be hit, return negative
	if (!hitsToKill || (ge->hits == 255)) return -1;

	// If the event has already been destroyed, do nothing
	if (ge->hits >= hitsToKill) return 0;

	// Check if the event has been killed
	if (ge->hits + hits >= hitsToKill) {

		// Notify the player that shot the bullet
		// If this returns false, ignore the hit
		if (!source->takeEvent(eventSet + ge->event, gridX, gridY, ticks)) {

			return hitsToKill - ge->hits;

		}

		ge->hits = (hits == 255)? 255: hitsToKill;
		ge->time = time;

	} else {

		ge->hits += hits;

	}

	if (multiplayer) {

		unsigned char buffer[MTL_L_GRID];
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
void JJ1Level::setEventTime (unsigned char gridX, unsigned char gridY, unsigned int time) {

	grid[gridY][gridX].time = time;

}


/**
 * Get a sprite.
 *
 * @param sprite Sprite number
 *
 * @return Sprite
 */
Sprite* JJ1Level::getSprite (unsigned char sprite) {

	return spriteSet + sprite;

}


/**
 * Get an animation.
 *
 * @param anim Animation number
 *
 * @return Animation
 */
Anim* JJ1Level::getAnim (unsigned char anim) {

	return animSet + anim;

}


/**
 * Get a "miscellaneous" animation.
 *
 * @param anim Animation number
 *
 * @return Animation
 */
Anim* JJ1Level::getMiscAnim (unsigned char anim) {

	return animSet + miscAnims[anim];

}


/**
 * Get a player animation.
 *
 * @param anim Animation number
 *
 * @return Animation
 */
Anim* JJ1Level::getPlayerAnim (unsigned char anim) {

	return animSet + playerAnims[anim];

}

/**
 * Get a level animation.
 *
 * @param anim Animation number
 *
 * @return Animation
 */
Anim* JJ1Level::getLevelAnim (unsigned char anim) {

	return animSet + levelAnims[anim];

}

/**
 * Get a level sound.
 *
 * @param sound Sound number
 *
 * @return Sound
 */
SE::Type JJ1Level::getLevelSound (unsigned char sound) {

	return static_cast<SE::Type>(levelSounds[sound]);

}


/**
 * Set the water level.
 *
 * @param gridY New water level y-coordinate
 */
void JJ1Level::setWaterLevel (unsigned char gridY) {

	waterLevelTarget = TTOF(gridY) + F2;

	if (multiplayer) {

		unsigned char buffer[MTL_L_PROP];
		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 1; // set water level
		buffer[3] = gridY;
		buffer[4] = 0; // Doesn't really matter

		game->send(buffer);

	}

}


/**
 * Determine the water level.
 *
 * @return The y-coordinate of the water level
 */
fixed JJ1Level::getWaterLevel () {

	return waterLevel;

}


/**
 * Create new bullet(s) (or event(s), if applicable)
 *
 * @param sourcePlayer The player that fired the bullet (if any)
 * @param gridX The grid X-coordinate of the origin of the event that fired the bullet (if any)
 * @param gridY The grid Y-coordinate of the origin of the event that fired the bullet (if any)
 * @param startX The initial X-coordinate of the bullet
 * @param startY The initial Y-coordinate of the bullet
 * @param bullet Type
 * @param facing The direction of the bullet
 * @param time Time
 */
void JJ1Level::createBullet (JJ1LevelPlayer* sourcePlayer, unsigned char gridX, unsigned char gridY, fixed startX, fixed startY, unsigned char bullet, bool facing, unsigned int time) {

	signed char* set;
	int direction;

	set = bulletSet[bullet];

	direction = facing? 1: 0;

	if (set[B_GRAVITY | direction] == 4) {

		events = new JJ1StandardEvent(eventSet + set[B_SPRITE | direction], gridX, gridY, startX, startY + F32);

	} else if (set[B_SPRITE | direction] != 0) {

		// Create new bullet
		level->bullets = new JJ1Bullet(level->bullets,
			sourcePlayer,
			startX,
			startY,
			set,
			direction,
			time);

		if (set[B_XSPEED | direction | 2] != 0) {

			// Create the other bullet
			level->bullets = new JJ1Bullet(level->bullets,
				sourcePlayer,
				startX,
				startY,
				set,
				direction | 2,
				time);

		}

		// FIXME: rewrite "set"
		auto se = static_cast<SE::Type>(set[B_STARTSOUND]);
		playSound(se);

	}

}


/**
 * Start a flash palette effect.
 *
 * @param red Red component of flash colour
 * @param green Green component of flash colour
 * @param blue Blue component of flash colour
 * @param duration Duration of the flash effect
 */
void JJ1Level::flash (unsigned char red, unsigned char green, unsigned char blue, int duration) {

	paletteEffects = new FlashPaletteEffect(red, green, blue, duration, paletteEffects);

}


/**
 * Play the bonus level.
 *
 * @return Error code
 */
int JJ1Level::playBonus () {

	char *bonusFile;
	int ret;

	if (!localPlayer->getJJ1LevelPlayer()->hasGem()) return E_NONE;

	delete paletteEffects;
	paletteEffects = NULL;

	bonusFile = createFileName("BONUSMAP", worldToEpisode(worldNum));

	// If the gem has been collected, play the bonus level
	ret = game->playLevel(bonusFile);

	delete[] bonusFile;

	return ret;

}


/**
 * Interpret data received from client/server
 *
 * @param buffer Received data
 */
void JJ1Level::receive (unsigned char* buffer) {

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

}


/**
 * Play the level.
 *
 * @return Error code
 */
int JJ1Level::play () {
	JJ1LevelPlayer* levelPlayer = localPlayer->getJJ1LevelPlayer();
	tickOffset = globalTicks;
	ticks = T_STEP;
	steps = 0;
	bool pmessage = false, pmenu = false;
	int option = 0;

	unsigned int returnTime = 0;
	int timeBonus = -1;
	int perfect = 0, enemyPercent = 0, itemPercent = 0;

	video.setPalette(palette);

	playMusic(musicFile);

	while (true) {

		int ret = loop(pmenu, option, pmessage);
		if (ret < 0) return ret;

		// Check if level has been won
		if (game && returnTime && (ticks > returnTime)) {

			// Play "Level end" cutscene
			// if available and secret level 2 was not found (FIXME: check JJ1, if true)
			if (strlen(sceneFile) && nextLevelNum != 2) {
				if (playScene(sceneFile) == E_QUIT) return E_QUIT;
			}

			if (!multiplayer) {
				// If the gem has been collected, play the bonus level
				ret = playBonus();

				if (ret < 0) return ret;
			}

			// No next level, end game
			if (nextLevelNum == 99) {
				ret = game->setLevel(nullptr);
			} else {
				// Advance to next level
				char *string = createFileName("LEVEL", nextLevelNum, nextWorldNum);
				ret = game->setLevel(string);
				delete[] string;
			}

			if (ret < 0) return ret;

			return WON;
		}


		// Process frame-by-frame activity

		while (getTimeChange() >= T_STEP) {

			bool playerWasAlive = (localPlayer->getJJ1LevelPlayer()->getEnergy() != 0);

			// Apply controls to local player
			for (int i = 0; i < PCONTROLS; i++)
				localPlayer->setControl(i, controls.getState(i));

			ret = step();
			steps++;

			if (ret) return ret;

			if (!multiplayer && playerWasAlive && (localPlayer->getJJ1LevelPlayer()->getEnergy() == 0))
				flash(0, 0, 0, T_END << 1);

		}


		// Draw the graphics

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			font->showString("pause", (canvasW >> 1) - 44, 32);

		// If paused, silence music
		pauseMusic(pmessage && !pmenu);

		if (stage == LS_END) {

			// The level is over, so draw play statistics & bonuses

			// Apply time bonus

			if (timeBonus) {

				int bonusCount = (ticks - prevTicks) / 100;
				if (!bonusCount) bonusCount = 1;

				if (timeBonus == -1) {

					if (ticks < endTime) timeBonus = ((endTime - ticks) / 60000) * 100;
					else timeBonus = 0;

					LOG_DEBUG("Killed %d of %d enemies, needed %d",
						levelPlayer->getEnemies(), enemies, nEnemies[game->getDifficulty()]);
					LOG_DEBUG("Collected %d of %d items, needed %d",
						levelPlayer->getItems(), items, nItems);

					if (nEnemies[game->getDifficulty()]) {
						enemyPercent = (levelPlayer->getEnemies() * 100) / (float) nEnemies[game->getDifficulty()];
						if (enemyPercent > 100) enemyPercent = 100;
					}
					if (nItems) {
						itemPercent = (levelPlayer->getItems() * 100) / (float) nItems;
						if (itemPercent > 100) itemPercent = 100;
					}

					// FIXME: check if JJ1 shows perfect score when there are no enemies or items
					if (enemyPercent == 100 && itemPercent == 100) {
						perfect = 100;
						LOG_DEBUG("Reached a perfect (100%%) score");
					}

				} else if (timeBonus - bonusCount >= 0) {

					localPlayer->addScore(bonusCount * 10);
					timeBonus -= bonusCount;

					auto se = getLevelSound(LSND_ENDCOUNT);
					if(!isSoundPlaying(se))
						playSound(se);

				} else {

					localPlayer->addScore(timeBonus * 10);
					timeBonus = 0;

				}

				if (timeBonus == 0) {

					returnTime = ticks + T_END;
					paletteEffects = new WhiteOutPaletteEffect(T_END, paletteEffects);
					playSound(getLevelSound(LSND_DONE));

				}

			}

			// Display statistics & bonuses

			font->showString("time", (canvasW >> 1) - 152, (canvasH >> 1) - 60);
			font->showNumber(timeBonus, (canvasW >> 1) + 124, (canvasH >> 1) - 60);

			font->showString("enemies", (canvasW >> 1) - 152, (canvasH >> 1) - 40);
			font->showNumber(enemyPercent, (canvasW >> 1) + 124, (canvasH >> 1) - 40);
			font->showString("%", (canvasW >> 1) + 124, (canvasH >> 1) - 40);

			font->showString("items", (canvasW >> 1) - 152, (canvasH >> 1) - 20);
			font->showNumber(itemPercent, (canvasW >> 1) + 124, (canvasH >> 1) - 20);
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
