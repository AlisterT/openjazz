
/**
 *
 * @file menu.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 3rd February 2009: Created menu.h from parts of OpenJazz.h
 * - 21st July 2013: Created setup.h from parts of menu.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _MENU_H
#define _MENU_H


#include "game/gamemode.h"
#include "io/file.h"

#include "OpenJazz.h"


// Constants

#define ESCAPE_STRING "(esc) quits"

// Demo timeout
#define T_DEMO 20000


// Classes

/// Menu base class, providing generic menu screens
class Menu {

	protected:
		void showEscString ();
		int  message       (const char* text);
		int  generic       (const char** optionNames, int options, int& chosen);
		int  textInput     (const char* request, char*& text);

};

/// New game menus
class GameMenu : public Menu {

	private:
		SDL_Surface*  episodeScreens[11]; ///< Episode images
		SDL_Surface*  difficultyScreen; ///< 4 difficulty images
		SDL_Color     palette[256]; ///< Episode selection palette
		SDL_Color     greyPalette[256]; ///< Greyed-out episode selection palette
		int           episodes; ///< Number of episodes
		unsigned char difficulty; ///< Difficulty setting (0 = easy, 1 = medium, 2 = hard, 3 = turbo (hard in JJ2 levels))

		int playNewGame       (GameModeType mode, char* firstLevel);
		int newGameDifficulty (GameModeType mode, char* firstLevel);
		int newGameDifficulty (GameModeType mode, int levelNum, int worldNum);
		int newGameLevel      (GameModeType mode);
		int selectEpisode     (GameModeType mode, int episode);
		int newGameEpisode    (GameModeType mode);
		int joinGame          ();

	public:
		GameMenu  (File* file);
		~GameMenu ();

		int newGame  ();
		int loadGame ();

};

/// Setup menus
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
		int setupMain ();

};

/// Main menu
class MainMenu : public Menu {

	private:
		SDL_Surface* background; ///< Menu image
		SDL_Surface* highlight; ///< Menu image with highlighted text
		SDL_Surface* logo; ///< OJ logo image
		GameMenu*    gameMenu; ///< New game menu
		SDL_Color    palette[256]; ///< Menu palette

		int select (int option);

	public:
		MainMenu  ();
		~MainMenu ();

		int main ();

};


// Variables

EXTERN SDL_Color menuPalette[256]; /// Palette used by most menu screens

#endif

