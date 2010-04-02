
/*
 *
 * gamemode.h
 *
 * 2nd August 2009: Created gamemode.h from parts of game.h
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


#ifndef _GAMEMODE_H
#define _GAMEMODE_H


#include "io/network.h"


// Constants

#define MAX_PLAYERS (MAX_CLIENTS + 1)


// Enum

enum GameModeType {

	M_SINGLE = 0, M_COOP = 1, M_BATTLE = 2, M_TEAMBATTLE = 3, M_RACE = 4

};


// Classes

class Player;

class GameMode {

	public:
		virtual GameModeType  getMode    () = 0;
		virtual unsigned char chooseTeam () = 0;
		virtual void          drawScore  () = 0;
		virtual bool          hit        (Player *source, Player *victim);
		virtual bool          kill       (Player *source, Player *victim);
		virtual bool          endOfLevel (Player *player, unsigned char gridX, unsigned char gridY);
		virtual void          outOfTime  ();

};

class CooperativeGameMode : public GameMode {

	public:
		unsigned char chooseTeam ();
		virtual void  drawScore  ();

};

class FreeForAllGameMode : public GameMode {

	public:
		unsigned char chooseTeam ();
		virtual void  drawScore  ();

};

class TeamGameMode : public GameMode {

	public:
		unsigned char chooseTeam ();
		virtual void  drawScore  ();

};

class CoopGameMode : public CooperativeGameMode {

	public:
		GameModeType getMode    ();
		bool         endOfLevel (Player *player, unsigned char gridX, unsigned char gridY);

};

class BattleGameMode : public FreeForAllGameMode {

	private:
		int targetKills;

	public:
		GameModeType getMode ();

};

class TeamBattleGameMode : public TeamGameMode {

	private:
		int targetKills;

	public:
		GameModeType getMode ();

};

class RaceGameMode : public FreeForAllGameMode {

	private:
		int targetLaps;

	public:
		GameModeType getMode    ();
		bool         hit        (Player *source, Player *victim);
		bool         endOfLevel (Player *player, unsigned char gridX, unsigned char gridY);

};


// Variable

EXTERN GameMode *gameMode; // NULL for single-player games


// Function

GameMode * createGameMode (GameModeType mode);


#endif

