
/**
 *
 * @file player.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 31st January 2006: Created player.h from parts of OpenJazz.h
 * 24th June 2010: Created levelplayer.h from parts of player.h
 * 24th June 2010: Created bonusplayer.h from parts of player.h
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _PLAYER_H
#define _PLAYER_H


#include "baselevel.h"


// Constants

// Player defaults
#define CHAR_NAME  "jazz"
#define CHAR_FUR   4
#define CHAR_BAND  3
#define CHAR_GUN   2
#define CHAR_WBAND 8

// General
#define PCONTROLS   8 /* Number of player controls. */


// Enums

/// Player colours
enum PlayerColour {

	PC_GREY   = 0,
	PC_SGREEN = 1,
	PC_BLUE   = 2,
	PC_RED    = 3,
	PC_LGREEN = 4,
	PC_LEVEL1 = 5,
	PC_YELLOW = 6,
	PC_LEVEL2 = 7,
	PC_ORANGE = 8,
	PC_LEVEL3 = 9,
	PC_LEVEL4 = 10,
	PC_SANIM  = 11,
	PC_LANIM  = 12,
	PC_LEVEL5 = 13

};

/// Player event types
enum PlayerEvent {

	LPE_NONE, LPE_SPRING, LPE_FLOAT, LPE_PLATFORM

};


// Classes

class Anim;
class LevelPlayer;
class BonusPlayer;
class JJ2LevelPlayer;

/// Game player
class Player {

	private:
		Game*           game;
		LevelPlayer*    levelPlayer; ///< JJ1 level player
		BonusPlayer*    bonusPlayer; ///< JJ1 bonus level player
		JJ2LevelPlayer* jj2LevelPlayer; ///< JJ2 level player
		char*           name; ///< Name
		bool            pcontrols[PCONTROLS]; ///< Control status
		unsigned char   cols[PCOLOURS]; ///< Character colours
		int             ammo[4]; ///< Amount of ammo
		int             ammoType; ///< Ammo type. -1 = blaster, 0 = toaster, 1 = missiles, 2 = bouncer 3 = TNT
		int             score; ///< Total score
		int             lives; ///< Remaining lives
		int             fireSpeed; ///< Rapid-fire rate
		int             flockSize; ///< Number of accompanying birds
		unsigned char   team; ///< Team ID

		void addAmmo (int type, int amount);

	public:
		int teamScore; ///< Team's total score

		Player  ();
		~Player ();

		void            init              (Game* owner, char* playerName, unsigned char* cols, unsigned char newTeam);
		void            deinit            ();
		void            reset             (unsigned char x, unsigned char y);
		void            reset             (LevelType levelType, Anim** anims, unsigned char x, unsigned char y);

		void            addLife           ();
		void            addScore          (int addedScore);
		bool            endOfLevel        (unsigned char gridX, unsigned char gridY);
		int             getAmmo           (bool amount);
		BonusPlayer*    getBonusPlayer    ();
		unsigned char*  getCols           ();
		bool            getControl        (int control);
		JJ2LevelPlayer* getJJ2LevelPlayer ();
		LevelPlayer*    getLevelPlayer    ();
		int             getLives          ();
		char*           getName           ();
		int             getScore          ();
		unsigned char   getTeam           ();
		bool            hit               (Player* source);
		bool            kill              (Player* source);
		void            setCheckpoint     (unsigned char gridX, unsigned char gridY);
		void            setControl        (int control, bool state);

		void            send              (unsigned char* buffer);
		void            receive           (unsigned char* buffer);

		friend class LevelPlayer;
		friend class JJ2LevelPlayer;

};


// Variables

EXTERN Player* players;
EXTERN Player* localPlayer;
EXTERN int     nPlayers;

#endif

