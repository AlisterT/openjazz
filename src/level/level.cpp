
/*
 *
 * level.cpp
 *
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
#include "menu/menu.h"
#include "player/player.h"
#include "scene/scene.h"
#include "loop.h"
#include "util.h"

#include <string.h>


Level::Level () {

	// Do nothing

	return;

}


Level::Level (char* fileName, unsigned char diff, bool checkpoint) {

	int ret;

	// Load level data

	ret = load(fileName, diff, checkpoint);

	if (ret < 0) throw ret;

	// Arbitrary initial value
	smoothfps = 50.0f;

	return;

}


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

	return;

}


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


bool Level::checkMaskDown (fixed x, fixed y) {

	// Anything off the edge of the map is solid
	if ((x < 0) || (y < 0) || (x > TTOF(LW)) || (y > TTOF(LH)))
		return true;

	// Check the mask in the tile in question
	return mask[grid[FTOT(y)][FTOT(x)].tile][((y >> 9) & 56) + ((x >> 12) & 7)];

}


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


void Level::setNext (int nextLevel, int nextWorld) {

	unsigned char buffer[MTL_L_PROP];

	nextLevelNum = nextLevel;
	nextWorldNum = nextWorld;

	if (gameMode) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 0; // set next level
		buffer[3] = nextLevel;
		buffer[4] = nextWorld;

		game->send(buffer);

	}

	return;

}


void Level::setTile (unsigned char gridX, unsigned char gridY,
	unsigned char tile) {

	unsigned char buffer[MTL_L_GRID];

	grid[gridY][gridX].tile = tile;

	if (gameMode) {

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


signed char* Level::getEvent (unsigned char gridX, unsigned char gridY) {

	int event = grid[gridY][gridX].event;

	if (event) return eventSet[grid[gridY][gridX].event];

	return NULL;

}


unsigned char Level::getEventHits (unsigned char gridX, unsigned char gridY) {

	return grid[gridY][gridX].hits;

}


unsigned int Level::getEventTime (unsigned char gridX, unsigned char gridY) {

	return grid[gridY][gridX].time;

}


void Level::clearEvent (unsigned char gridX, unsigned char gridY) {

	unsigned char buffer[MTL_L_GRID];

	// Ignore if the event has been un-destroyed
	if (!grid[gridY][gridX].hits &&
		eventSet[grid[gridY][gridX].event][E_HITSTOKILL]) return;

	grid[gridY][gridX].event = 0;

	if (gameMode) {

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


int Level::hitEvent (unsigned char gridX, unsigned char gridY, Player* source) {

	GridElement* ge;
	unsigned char buffer[MTL_L_GRID];
	int hitsToKill;

	ge = grid[gridY] + gridX;

	hitsToKill = eventSet[ge->event][E_HITSTOKILL];

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

	if (gameMode) {

		buffer[0] = MTL_L_GRID;
		buffer[1] = MT_L_GRID;
		buffer[2] = gridX;
		buffer[3] = gridY;
		buffer[4] = 3; // hits variable
		buffer[5] = ge->hits;

		game->send(buffer);

	}

	// Return the number of hits remaining until the event is destroyed
	return hitsToKill - ge->hits;

}


void Level::setEventTime (unsigned char gridX, unsigned char gridY,
	unsigned int time) {

	grid[gridY][gridX].time = time;

	return;

}


signed char* Level::getBullet (unsigned char bullet) {

	return bulletSet[bullet];

}


Sprite* Level::getSprite (unsigned char sprite) {

	return spriteSet + sprite;

}


Anim* Level::getAnim (unsigned char anim) {

	return animSet + anim;

}


Anim* Level::getMiscAnim (unsigned char anim) {

	return animSet + miscAnims[anim];

}


void Level::setWaterLevel (unsigned char gridY) {

	unsigned char buffer[MTL_L_PROP];

	waterLevelTarget = TTOF(gridY);

	if (gameMode) {

		buffer[0] = MTL_L_PROP;
		buffer[1] = MT_L_PROP;
		buffer[2] = 1; // set water level
		buffer[3] = gridY;
		buffer[4] = 0; // Doesn't really matter

		game->send(buffer);

	}

	return;

}


fixed Level::getWaterLevel () {

	return waterLevel;

}


void Level::playSound (int sound) {

	if (sound > 0) ::playSound(soundMap[sound - 1]);

	return;

}


void Level::flash (unsigned char red, unsigned char green, unsigned char blue, int duration) {

	paletteEffects = new FlashPaletteEffect(red, green, blue, duration, paletteEffects);

	return;

}


void Level::setStage (LevelStage newStage) {

	unsigned char buffer[MTL_L_STAGE];

	if (stage == newStage) return;

	stage = newStage;

	if (gameMode) {

		buffer[0] = MTL_L_STAGE;
		buffer[1] = MT_L_STAGE;
		buffer[2] = stage;
		game->send(buffer);

	}

	return;

}


LevelStage Level::getStage () {

	return stage;

}


int Level::playBonus () {

	Bonus *bonus;
	char *bonusFile;
	int ret;

	if (!localPlayer->hasGem()) return E_NONE;

	delete paletteEffects;
	paletteEffects = NULL;

	bonusFile = createFileName(F_BONUSMAP, 0);

	try {

		bonus = new Bonus(bonusFile, difficulty);

	} catch (int e) {

		return e;

	}

	delete[] bonusFile;

	ret = bonus->play();

	delete bonus;

	if (ret == E_NONE) playMusic("menusng.psm");

	return ret;

}


void Level::receive (unsigned char* buffer) {

	// Interpret data received from client/server

	switch (buffer[1]) {

		case MT_L_PROP:

			if (buffer[2] == 0) {

				nextLevelNum = buffer[3];
				nextWorldNum = buffer[4];

			} else if (buffer[2] == 1) {

				waterLevelTarget = TTOF(buffer[3]);

			} else if (buffer[2] == 2) {

				if (stage == LS_NORMAL)
					endTime += 2 * 60 * 1000; // 2 minutes. Is this right?

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


int Level::play () {

	const char* options[5] =
		{"continue game", "save game", "load game", "setup options", "quit game"};
	char *string;
	bool pmessage, pmenu;
	int stats, option;
	unsigned int returnTime;
 	int perfect;
 	int timeBonus;
 	int count;


	tickOffset = globalTicks;
	ticks = 16;
	prevStepTicks = 0;

	pmessage = pmenu = false;
	option = 0;
	stats = 0;

	returnTime = 0;
	timeBonus = -1;
	perfect = 0;

	video.setPalette(palette);

	playMusic(musicFile);

	while (true) {

		if (loop(NORMAL_LOOP, paletteEffects) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) {

			pmenu = !pmenu;
			option = 0;

		}

		if (controls.release(C_PAUSE)) pmessage = !pmessage;

		if (controls.release(C_STATS)) {

			if (!gameMode) stats ^= S_SCREEN;
			else stats = (stats + 1) & 3;

		}

		if (pmenu) {

			// Deal with menu controls

			if (controls.release(C_UP)) option = (option + 4) % 5;

			if (controls.release(C_DOWN)) option = (option + 1) % 5;

			if (controls.release(C_ENTER)) {

				switch (option) {

					case 0: // Continue

						pmenu = false;

						break;

					case 1: // Save

						break;

					case 2: // Load

						break;

					case 3: // Setup

						if (!gameMode) {

							if (menu->setup() == E_QUIT) return E_QUIT;

							// Restore level palette
							video.setPalette(palette);

						}

						break;

					case 4: // Quit game

						return E_NONE;

				}

			}

		}

		if (!gameMode) paused = pmessage || pmenu;

		timeCalcs();


		// Check if level has been won
		if (game && returnTime && (ticks > returnTime)) {

			if (!gameMode) {

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

		if (!paused && (ticks >= prevStepTicks + 16)) {

			// Apply controls to local player
			for (count = 0; count < PCONTROLS; count++)
				localPlayer->setControl(count, controls.getState(count));

			count = step();

			if (count) return count;

		}


		// Draw the graphics

		draw();


		// If paused, draw "PAUSE"
		if (pmessage && !pmenu)
			fontsFont->showString("pause", (canvasW >> 1) - 44, 32);


		// If this is a competitive game, draw the score
		if (gameMode) gameMode->drawScore();


		// Draw statistics
		drawStats(stats, BLACK);


		if (stage == LS_END) {

			// The level is over, so draw play statistics & bonuses

			// Apply time bonus

			if (timeBonus) {

				count = (ticks - prevTicks) / 100;
				if (!count) count = 1;

				if (timeBonus == -1) {

					timeBonus = ((endTime - ticks) / 60000) * 100;

					if (timeBonus < 0) timeBonus = 0;

					if ((localPlayer->getEnemies() == enemies) &&
						(localPlayer->getItems() == items)) perfect = 100;

				} else if (timeBonus - count >= 0) {

					localPlayer->addScore(count);
					timeBonus -= count;

				} else {

					localPlayer->addScore(timeBonus);
					timeBonus = 0;

				}

				if (timeBonus == 0) {

					returnTime = ticks + T_END;
					paletteEffects = new WhiteOutPaletteEffect(T_END, paletteEffects);
					::playSound(S_UPLOOP);

				}

			}

			// Display statistics & bonuses
			// TODO: Display percentage symbol

			fontsFont->showString("time", (canvasW >> 1) - 152, (canvasH >> 1) - 60);
			fontsFont->showNumber(timeBonus, (canvasW >> 1) + 124, (canvasH >> 1) - 60);

			fontsFont->showString("enemies", (canvasW >> 1) - 152, (canvasH >> 1) - 40);

			if (enemies)
				fontsFont->showNumber((localPlayer->getEnemies() * 100) / enemies, (canvasW >> 1) + 124, (canvasH >> 1) - 40);
			else
				fontsFont->showNumber(0, (canvasW >> 1) + 124, (canvasH >> 1) - 40);
			fontsFont->showString("%", (canvasW >> 1) + 124, (canvasH >> 1) - 40);

			fontsFont->showString("items", (canvasW >> 1) - 152, (canvasH >> 1) - 20);

			if (items)
				fontsFont->showNumber((localPlayer->getItems() * 100) / items, (canvasW >> 1) + 124, (canvasH >> 1) - 20);
			else
				fontsFont->showNumber(0, (canvasW >> 1) + 124, (canvasH >> 1) - 20);
			fontsFont->showString("%", (canvasW >> 1) + 124, (canvasH >> 1) - 20);

			fontsFont->showString("perfect", (canvasW >> 1) - 152, canvasH >> 1);
			fontsFont->showNumber(perfect, (canvasW >> 1) + 124, canvasH >> 1);

			fontsFont->showString("score", (canvasW >> 1) - 152, (canvasH >> 1) + 40);
			fontsFont->showNumber(localPlayer->getScore(), (canvasW >> 1) + 124, (canvasH >> 1) + 40);

		}


		if (pmenu) {

			// Draw the menu

			drawRect((canvasW >> 2) - 8, (canvasH >> 1) - 46, 144, 92, BLACK);

			for (count = 0; count < 5; count++) {

				if (count == option) fontmn2->mapPalette(240, 8, 47, -16);
				else fontmn2->mapPalette(240, 8, 15, -16);

				fontmn2->showString(options[count], canvasW >> 2, (canvasH >> 1) + (count << 4) - 38);

			}

			fontmn2->restorePalette();

		}


		// Networking

		if (gameMode) {

			count = game->step(ticks);

			switch (count) {

				case E_UNUSED:

					return E_NONE;

				case E_NONE:

					break;

				default:

					return count;

			}

		}

	}

	return E_NONE;

}


