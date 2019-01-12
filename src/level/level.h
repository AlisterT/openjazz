
/**
 *
 * @file level.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 30th March 2010: Created baselevel.h from parts of level.h
 * - 1st August 2012: Renamed baselevel.h to level.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _BASELEVEL_H
#define _BASELEVEL_H


#include "menu/menu.h"


// Macros

// For converting between tile positions and int/fixed values
#define FTOT(x) ((x) >> 15) ///< Fixed to Tile
#define TTOF(x) ((x) << 15) ///< Tile to Fixed
#define ITOT(x) ((x) >> 5)  ///< Int to Tile
#define TTOI(x) ((x) << 5)  ///< Tile to Int


// Constants

// Return values
#define WON  1
#define LOST 2

// Time interval
#define T_STEP 16


// Enums

/// Level type
enum LevelType {

	LT_JJ1 = 0, ///< JJ1 level
	LT_JJ1BONUS = 1, ///< JJ1 bonus level
	LT_JJ2 = 2 ///< JJ2 level

};

/// Which stats to display on-screen
enum LevelStats {

	S_PLAYERS = 1, ///< Display player list
	S_SCREEN = 2 ///< Display video statisitics

};

/// Level stage
enum LevelStage {

	LS_NORMAL = 0, ///< Normal gameplay
	LS_SUDDENDEATH = 1, ///< Sudden death
	LS_END = 2 ///< Ending sequence

};


// Classes

class Anim;
class File;
class Game;
class PaletteEffect;
class Sprite;

/// Base class for all level classes
class Level {

	private:
		const char* menuOptions[6];
		SetupMenu   setupMenu; ///< Setup menu to run on the player's command

		int select (bool& menu, int option);

	protected:
		Game*          game;
		PaletteEffect* paletteEffects; ///< Palette effects in use while playing the level
		SDL_Color      palette[256]; ///< Palette in use while playing the level
		int            sprites; ///< The number of sprite that have been loaded
		unsigned int   tickOffset; ///< Level time offset from system time
		unsigned int   steps; ///< Number of steps taken
		unsigned int   prevTicks; ///< Time the last visual update started
		unsigned int   ticks; ///< Current time
		unsigned int   endTime; ///< Tick at which the level will end
		float          smoothfps; ///< Smoothed FPS counter
		int            items; ///< Number of items to be collected
		bool           multiplayer; ///< Whether or not this is a multiplayer game
		bool           paused; ///< Whether or not the level is paused
		LevelStage     stage; ///< Level stage
		int            stats; ///< Which statistics to display on-screen, see #LevelStats

		void createLevelPlayers (LevelType levelType, Anim** anims, Anim** flippedAnims, bool checkpoint, unsigned char x, unsigned char y);

		int  playScene     (const char* file);
		void timeCalcs     ();
		int  getTimeChange ();
		void drawOverlay   (unsigned char bg, bool menu, int option,
			unsigned char textPalIndex, unsigned char selectedTextPalIndex,
			int textPalSpan);
		int  loop          (bool& menu, int& option, bool& message);

	public:
		Level          (Game* owner);
		virtual ~Level ();

		void         addTimer (int seconds);
		LevelStage   getStage ();
		void         setStage (LevelStage stage);
		virtual void receive  (unsigned char* buffer) = 0;

};


// Variables

EXTERN fixed      viewX, viewY; ///< Level viewing co-ordinates

#endif

