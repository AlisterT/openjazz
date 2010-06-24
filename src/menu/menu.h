
/*
 *
 * menu.h
 *
 * 3rd February 2009: Created menu.h from parts of OpenJazz.h
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


#ifndef _MENU_H
#define _MENU_H


#include "game/gamemode.h"
#include "io/file.h"

#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constant

// Demo timeout
#define T_DEMO 20000


// Classes

class Menu {

	protected:
		int message   (const char* text);
		int generic   (const char** optionNames, int options, int& chosen);
		int textInput (const char* request, char*& text);

};


class GameMenu : public Menu {

	private:
		SDL_Surface*  episodeScreens[11];
		SDL_Surface*  difficultyScreen;
		SDL_Color     palette[256];
		SDL_Color     greyPalette[256];
		int           episodes;
		unsigned char difficulty;

		int newGameDifficulty (GameModeType mode, int levelNum, int worldNum);
		int newGameLevel      (GameModeType mode);
		int newGameEpisode    (GameModeType mode);
		int joinGame          ();

	public:
		GameMenu  (File* file);
		~GameMenu ();

		int newGame  ();
		int loadGame ();

};


class SetupMenu : public Menu {

	private:
		int setupKeyboard   ();
		int setupJoystick   ();
		int setupResolution ();
#ifdef SCALE
		int setupScaling    ();
#endif
		int setupSound      ();

	public:
		int setup ();

};


class MainMenu : public Menu {

	private:
		SDL_Surface* background;
		SDL_Surface* highlight;
		SDL_Surface* logo;
		GameMenu*    gameMenu;
		SDL_Color    palette[256];

	public:
		MainMenu  ();
		~MainMenu ();

		int main ();

};


// Variable

EXTERN SDL_Color menuPalette[256];

#endif

