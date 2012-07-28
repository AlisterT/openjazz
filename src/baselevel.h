
/**
 *
 * @file baselevel.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 30th March 2010: Created baselevel.h from parts of level.h
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
 */


#ifndef _BASELEVEL_H
#define _BASELEVEL_H


#include "io/gfx/paletteeffects.h"
#include "menu/menu.h"


// Macros

// For converting between tile positions and int/fixed values
#define FTOT(x) ((x) >> 15)
#define TTOF(x) ((x) << 15)
#define ITOT(x) ((x) >> 5)
#define TTOI(x) ((x) << 5)


// Enums

/// Level type
enum LevelType {

	LT_LEVEL, LT_BONUS, LT_JJ2LEVEL

};

/// Which stats to display on-screen
enum LevelStats {

	S_PLAYERS = 1, S_SCREEN = 2

};

/// Level stage
enum LevelStage {

	LS_NORMAL = 0, LS_SUDDENDEATH = 1, LS_END = 2

};


// Classes

class File;
class Sprite;

/// Base class for all level classes
class BaseLevel {

	private:
		char*     menuOptions[6];
		SetupMenu setupMenu; ///< Setup menu to run on the player's command

		int select (bool& menu, int option);

	protected:
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

		int  playScene     (char* file);
		void timeCalcs     ();
		int  getTimeChange ();
		void drawOverlay   (unsigned char bg, bool menu, int option,
			unsigned char textPalIndex, unsigned char selectedTextPalIndex,
			int textPalSpan);
		int  loop          (bool& menu, int& option, bool& message);

	public:
		BaseLevel          ();
		virtual ~BaseLevel ();

		void         addTimer (int seconds);
		LevelStage   getStage ();
		void         setStage (LevelStage stage);
		virtual void receive  (unsigned char* buffer) = 0;

};


// Variables

EXTERN BaseLevel* baseLevel; ///< Current level
EXTERN fixed      viewX, viewY; ///< Level viewing co-ordinates

#endif

