
/*
 *
 * menu.h
 *
 * Created on the 3rd of February 2009 from parts of OpenJazz.h
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
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


#include "OpenJazz.h"


// Constant

// Demo timeout
#define T_DEMO 20000

// Class

class Menu {

	private:
		SDL_Surface   *screens[15];
		int            episodes;
		unsigned char  difficulty;

		int message           (char *text);
		int generic           (char **optionNames, int options, int *chosen);
		int textInput         (char *request, char **text);
		int newGameDifficulty (int mode, int levelNum, int worldNum);
		int newGameLevel      (int mode);
		int newGameEpisode    (int mode);
		int joinGame          ();
		int loadGame          ();
		int setupKeyboard     ();
		int setupJoystick     ();
		int setupResolution   ();

	public:
		SDL_Color      palettes[4][256];

		Menu      ();
		~Menu     ();

		int setup ();
		int run   ();

};


// Variable

Extern Menu          *menu;

#endif

