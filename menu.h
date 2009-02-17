
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


// Class

class Menu {

	private:
		SDL_Surface   *screens[15];
		SDL_Color      palettes[4][256];
		char          *nextLevel;
		int            episodes;
		unsigned char  difficulty;

		int textInput         (char * request, char ** text);
		int newGameDifficulty ();
		int newGameLevel      ();
		int newGame           ();
		int loadGame          ();
		int setupCharacter    ();
		int setupKeyboard     ();
		int setupJoystick     ();
		int setupResolution   ();

	public:
		Menu              ();
		~Menu             ();
		int  run          ();
		int  setup        ();
		void setNextLevel (char *fn);

};


