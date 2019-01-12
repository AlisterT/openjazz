
/**
 *
 * @file game.h
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


#ifndef _GAME_H
#define _GAME_H


#include "gamemode.h"

#include "io/network.h"
#include "level/level.h"


// Constants

// Time intervals
#define T_SSEND   20
#define T_SCHECK  1000
#define T_CSEND   10
#define T_CCHECK  1000

// Message categories and types
#define MCMASK     0xF0
#define MC_GAME    0x00
#define MC_LEVEL   0x10
#define MC_PLAYER  0x20

#define MT_G_PROPS 0x00 /* Game properties */
#define MT_G_PJOIN 0x01 /* New player joined */
#define MT_G_PQUIT 0x02 /* Player left */
#define MT_G_LEVEL 0x03 /* Level data */
#define MT_G_CHECK 0x04
#define MT_G_SCORE 0x05 /* Team scored a roast/lap/etc. */
#define MT_G_LTYPE 0x06 /* Level type */

#define MT_L_PROP  0x10 /* Level property */
#define MT_L_GRID  0x11 /* Change to gridElement */
#define MT_L_STAGE 0x12 /* Change in level stage */

#define MT_P_ANIMS 0x20 /* Player animations */
#define MT_P_TEMP  0x21 /* Temporary player properties, e.g. position */

// Minimum message lengths, including header
#define MTL_G_PROPS 8
#define MTL_G_PJOIN 10
#define MTL_G_PQUIT 3
#define MTL_G_LEVEL 4 /* + amount of level data */
#define MTL_G_CHECK 6
#define MTL_G_SCORE 3
#define MTL_G_LTYPE 3

#define MTL_L_PROP  5
#define MTL_L_GRID  8
#define MTL_L_STAGE 3

#define MTL_P_ANIMS 3 /* + PANIMS, BPANIMS, or 1 (for JJ2) */
#define MTL_P_TEMP  46

#define BUFFER_LENGTH 255 /* Should always be big enough to hold any message */


// Classes

class Anim;
class File;

/// Base class for game handling classes
class Game {

	private:
		int planetId; ///< ID of last planet approach sequence

		bool isFileType (const char *fileName, const char *type, int typeLength);

	protected:
		GameMode*      mode; ///< Mode-specific management
		Level*         baseLevel; ///< Current level
		char*          levelFile; ///< Current level's file name
		LevelType      levelType; ///< Current level's type
		int            difficulty; ///< Difficulty setting (0 = easy, 1 = medium, 2 = hard, 3 = turbo (hard in JJ2 levels))
		unsigned int   sendTime; ///< The next time data will be sent
		unsigned int   checkTime; ///< The next time a connection/disconnection will be dealt with
		short int      checkX; ///< X-coordinate of the level checkpoint
		short int      checkY; ///< Y-coordinate of the level checkpoint

		Game ();

		GameMode* createMode (GameModeType modeType);

		LevelType getLevelType (const char* fileName);
		int       playLevel    (char *fileName, bool intro, bool checkpoint);

		void addLevelPlayer (Player *player);

	public:
		virtual ~Game ();

		GameMode*    getMode       ();
		int          getDifficulty ();
		void         setDifficulty (int diff);
		int          playLevel     (char *fileName);
		virtual int  setLevel      (char *fileName) = 0;
		int          play          ();
		void         view          (int change);
		virtual void send          (unsigned char *buffer) = 0;
		virtual int  step          (unsigned int ticks) = 0;
		virtual void score         (unsigned char team) = 0;
		virtual void setCheckpoint (int gridX, int gridY) = 0;
		void         resetPlayer   (Player *player);

};


/// Game handling for single-player local play
class LocalGame : public Game {

	public:
		LocalGame  (const char *firstLevel, int gameDifficulty);
		~LocalGame ();

		int  setLevel      (char *fileName);
		void send          (unsigned char *buffer);
		int  step          (unsigned int ticks);
		void score         (unsigned char team);
		void setCheckpoint (int gridX, int gridY);

};


/// Game handling for multiplayer servers
class ServerGame : public Game {

	private:
		int            clientStatus[MAX_CLIENTS]; /**< Array of client statuses
 			-2: Connected and operational
 			-1: Not connected
			>=0: Number of bytes of the level that have been sent */
		int            clientPlayer[MAX_CLIENTS]; ///< Array of client player indexes
		int            clientSock[MAX_CLIENTS]; ///< Array of client sockets
		unsigned char  recvBuffers[MAX_CLIENTS][BUFFER_LENGTH]; ///< Array of buffers containing data received from clients
		int            received[MAX_CLIENTS]; ///< Array containing the amount of data received from each client
		unsigned char *levelData; ///< Contents of the current level file
		int            levelSize; ///< Size of the current level file
		int            sock; ///< Server socket

	public:
		ServerGame         (GameModeType mode, char *firstLevel, int gameDifficulty);
		~ServerGame        ();

		int  setLevel      (char *fileName);
		void send          (unsigned char *buffer);
		int  step          (unsigned int ticks);
		void score         (unsigned char team);
		void setCheckpoint (int gridX, int gridY);

};


/// Game handling for multiplayer clients
class ClientGame : public Game {

	private:
		File          *file; ///< File to which the incoming level will be written
		unsigned char  recvBuffer[BUFFER_LENGTH]; ///< Buffer containing data received from server
		int            received; ///< Amount of data received from server
		int            clientID; ///< Client's index on the server
		int            maxPlayers; ///< The maximum number of players in the game
		int            sock; ///< Client socket

	public:
		ClientGame         (char *address);
		~ClientGame        ();

		int  setLevel      (char *fileName);
		void send          (unsigned char *buffer);
		int  step          (unsigned int ticks);
		void score         (unsigned char team);
		void setCheckpoint (int gridX, int gridY);

};

#endif

