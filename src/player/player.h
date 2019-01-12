
/**
 *
 * @file player.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created player.h from parts of OpenJazz.h
 * - 24th June 2010: Created levelplayer.h from parts of player.h
 * - 24th June 2010: Created bonusplayer.h from parts of player.h
 *
 * @par Licence:
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _PLAYER_H
#define _PLAYER_H


#include "level/level.h"


// Constants

// Player defaults
#define CHAR_NAME  "jazz"
#define CHAR_FUR   4
#define CHAR_BAND  3
#define CHAR_GUN   2
#define CHAR_WBAND 8

// General
#define PCONTROLS   8 /* Number of player controls. */

// Number of configurable player colour ranges
#define PCOLOURS 4


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


// Classes

class Anim;
class JJ1LevelPlayer;
class JJ1BonusLevelPlayer;
class JJ2LevelPlayer;
class LevelPlayer;

/// Game player
class Player {

	private:
		Game*                game;
		LevelPlayer*         levelPlayer; ///< Level player base class

		char*           name; ///< Name
		bool            pcontrols[PCONTROLS]; ///< Control status
		unsigned char   cols[PCOLOURS]; ///< Character colours
		int             ammo[5]; ///< Amount of ammo
		int             ammoType; ///< Ammo type. -1 = blaster, 0 = toaster, 1 = missiles, 2 = bouncer, 3 = unknown, 4 = TNT
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
		void            clearAmmo         ();
		void            reset             (int x, int y);

		void                 createLevelPlayer      (LevelType levelType, Anim** anims, Anim** flippedAnims, unsigned char x, unsigned char y);
		LevelPlayer*         getLevelPlayer         ();
		JJ1BonusLevelPlayer* getJJ1BonusLevelPlayer ();
		JJ1LevelPlayer*      getJJ1LevelPlayer      ();
		JJ2LevelPlayer*      getJJ2LevelPlayer      ();

		void            addLife           ();
		void            addScore          (int addedScore);
		bool            endOfLevel        (int gridX, int gridY);
		int             getAmmoType       ();
		int             getAmmo           ();
		unsigned char*  getCols           ();
		bool            getControl        (int control);
		int             getLives          ();
		char*           getName           ();
		int             getScore          ();
		unsigned char   getTeam           ();
		bool            hit               (Player* source);
		bool            kill              (Player* source);
		void            setCheckpoint     (int gridX, int gridY);
		void            setControl        (int control, bool state);

		void            send              (unsigned char* buffer);
		void            receive           (unsigned char* buffer);

		friend class JJ1LevelPlayer;
		friend class JJ2LevelPlayer;

};


// Variables

EXTERN Player* players;
EXTERN Player* localPlayer;
EXTERN int     nPlayers;

#endif

