
/*
 *
 * player.h
 *
 * 31st January 2006: Created player.h from parts of OpenJazz.h
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _PLAYER_H
#define _PLAYER_H


#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// Player colours
#define PC_WHITE     0
#define PC_SGREEN    16
#define PC_BLUE      23
#define PC_RED       32
#define PC_LGREEN    48
#define PC_LEVEL1    64
#define PC_YELLOW    75
#define PC_LEVEL2    80
#define PC_ORANGE    88
#define PC_LEVEL3    96
#define PC_LEVEL4    104
#define PC_SANIM     112
#define PC_LANIM     116
#define PC_LEVEL5    124

// Player defaults
#define CHAR_NAME   "jazz"
#define CHAR_FUR    PC_LGREEN
#define CHAR_BAND   PC_RED
#define CHAR_GUN    PC_BLUE
#define CHAR_WBAND  PC_ORANGE

// General
#define PANIMS     38 /* Number of player animations. Is probably higher. */
#define PCONTROLS   8 /* Number of player controls. */


// Classes

class Bird;
class BonusPlayer;
class LevelPlayer;

class Player {

	private:
		LevelPlayer*  levelPlayer;
		BonusPlayer*  bonusPlayer;
		Bird*         bird;
		char*         name;
		bool          pcontrols[PCONTROLS];
		SDL_Color     palette[256];
		unsigned char cols[4];
		int           ammo[4];
		int           ammoType; /* -1 = blaster, 0 = toaster, 1 = missiles, 2 = bouncer 3 = TNT */
		int           score;
		int           lives;
		int           fireSpeed;
		unsigned char team;

		void addAmmo (int type, int amount);

	public:
		int teamScore;

		Player  ();
		~Player ();

		void           init           (char* playerName, unsigned char* cols, unsigned char newTeam);
		void           deinit         ();
		void           reset          ();

		void           addLife        ();
		void           addScore       (int addedScore);
		int            getAmmo        (bool amount);
		BonusPlayer*   getBonusPlayer ();
		unsigned char* getCols        ();
		bool           getControl     (int control);
		LevelPlayer*   getLevelPlayer ();
		int            getLives       ();
		char*          getName        ();
		int            getScore       ();
		unsigned char  getTeam        ();
		void           setControl     (int control, bool state);

		void           send           (unsigned char* buffer);
		void           receive        (unsigned char* buffer);

		friend class LevelPlayer;

};


// Variables

EXTERN Player* players;
EXTERN Player* localPlayer;
EXTERN int     nPlayers;

// Configuration data
EXTERN char*         characterName;
EXTERN unsigned char characterCols[4];

#endif

