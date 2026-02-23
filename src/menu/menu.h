
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
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _MENU_H
#define _MENU_H


#include "game/gamemode.h"

#include "OpenJazz.h"

#ifdef OJ_SDL3
	#include <SDL3/SDL.h>
#else
	#include <SDL.h>
#endif

// Constants

#define ESCAPE_STRING "(esc) quits"

#define MAX_EPISODES 11

// Demo timeout
#define T_DEMO 20000


// Classes

class File;

/// Menu base class, providing generic menu screens
class Menu {

	protected:
		void showEscString (bool alignLeft = true);
		int  message       (const char* text);
		int  generic       (const char* title, const char** optionNames, int options, int& chosen);
		int  textInput     (const char* request, char*& text, bool ip = false);

};

/// Saving and loading menu
class FileMenu : public Menu {

	public:
		int main (bool forSaving, bool showCustom);

};

/// New game menus
class GameMenu : public Menu {

	private:
		SDL_Surface*  episodeScreens[MAX_EPISODES]; ///< Episode images
		SDL_Surface*  difficultyScreen; ///< 4 difficulty images
		SDL_Color     palette[MAX_PALETTE_COLORS]; ///< Episode selection palette
		SDL_Color     greyPalette[MAX_PALETTE_COLORS]; ///< Greyed-out episode selection palette
		int           episodes; ///< Number of episodes
		unsigned char difficulty; ///< Difficulty setting (0 = easy, 1 = medium, 2 = hard, 3 = turbo (hard in JJ2 levels))
		FileMenu*     fileMenu; ///< Load menu

		int playNewGame       (GameModeType mode, char* firstLevel);
		int newGameDifficulty (GameModeType mode, char* firstLevel);
		int newGameDifficulty (GameModeType mode, int levelNum, int worldNum);
		int newGameLevel      (GameModeType mode);
		int selectEpisode     (GameModeType mode, int episode);
		int newGameEpisode    (GameModeType mode);
		int joinGame          ();
		int loadGameCustom    ();

	public:
		explicit GameMenu(File* file);
		~GameMenu();

		int newGame  ();
		int loadGame ();
		int loadGame (int levelNum, int worldNum);

};

/// Setup menus
class SetupMenu : public Menu {

	private:
#ifndef NO_KEYBOARD_CFG
		int setupKeyboard ();
#endif
#ifndef NO_CONTROLLER_CFG
		int setupJoystick ();
#endif
#if !defined(NO_RESIZE) || defined(SCALE)
		int setupVideo    ();
#endif
		int setupAudio    ();

	public:
		int setupMain     ();

};

/// Main menu
class MainMenu : public Menu {

	private:
		SDL_Surface* background; ///< Menu image
		SDL_Surface* highlight; ///< Menu image with highlighted text
		SDL_Surface* logo; ///< OJ logo image
		GameMenu*    gameMenu; ///< New game menu
		SDL_Color    palette[MAX_PALETTE_COLORS]; ///< Menu palette

		MainMenu(const MainMenu&); // non construction-copyable
		MainMenu& operator=(const MainMenu&); // non copyable
		int select (int option);

	public:
		MainMenu  ();
		~MainMenu ();

		int main ();
		int skip (int levelNum, int worldNum);

};


// Variables

EXTERN SDL_Color menuPalette[MAX_PALETTE_COLORS]; /// Palette used by most menu screens

#endif

