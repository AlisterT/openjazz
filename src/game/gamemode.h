
/**
 *
 * @file gamemode.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 2nd March 2009: Created network.h from parts of OpenJazz.h
 * - 9th February 2009: Renamed network.h to game.h
 * - 2nd August 2009: Created gamemode.h from parts of game.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _GAMEMODE_H
#define _GAMEMODE_H


// Constants

#define MAX_PLAYERS (MAX_CLIENTS + 1)


// Enum

/// Game mode identifier
enum GameModeType {

	M_SINGLE = 0, ///< Single-player mode
	M_COOP = 1, ///< Co-operative mode
	M_BATTLE = 2, ///< Battle mode
	M_TEAMBATTLE = 3, ///< Team battle mode
	M_RACE = 4 ///< Race mode

};


// Classes

class Font;
class Game;
class Player;

/// Game mode base class
class GameMode {

	public:
        virtual ~GameMode ();

		virtual GameModeType  getMode    () = 0;
		virtual unsigned char chooseTeam () = 0;
		virtual void          drawScore  (Font* font) = 0;
		virtual bool          hit        (Player *source, Player *victim);
		virtual bool          kill       (Game* game, Player *source, Player *victim);
		virtual bool          endOfLevel (Game* game, Player *player, int gridX, int gridY);
		virtual void          outOfTime  ();

};

/// Single-player game mode
class SingleGameMode : public GameMode {

	public:
		GameModeType  getMode    ();
		unsigned char chooseTeam ();
		void          drawScore  (Font* font);

};

/// Co-operative game mode base class
class CooperativeGameMode : public GameMode {

	public:
		unsigned char chooseTeam ();
		virtual void  drawScore  (Font* font);

};

/// Free-for-all game mode base class
class FreeForAllGameMode : public GameMode {

	public:
		unsigned char chooseTeam ();
		virtual void  drawScore  (Font* font);

};

/// Team-based game mode base class
class TeamGameMode : public GameMode {

	public:
		unsigned char chooseTeam ();
		virtual void  drawScore  (Font* font);

};

/// Co-operative game mode
class CoopGameMode : public CooperativeGameMode {

	public:
		GameModeType getMode ();

};

/// Battle game mode
class BattleGameMode : public FreeForAllGameMode {

	private:
		int targetKills; ///< Number of kills required for a player to win

	public:
		GameModeType getMode ();

};

/// Team battle game mode
class TeamBattleGameMode : public TeamGameMode {

	private:
		int targetKills; ///< Number of kills required for a team to win

	public:
		GameModeType getMode ();

};

/// Race game mode
class RaceGameMode : public FreeForAllGameMode {

	private:
		int targetLaps; ///< Number of laps required for a player to win

	public:
		GameModeType getMode    ();
		bool         hit        (Player *source, Player *victim);
		bool         endOfLevel (Game* game, Player *player, int gridX, int gridY);

};

#endif

