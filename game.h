
/*
 *
 * game.h
 *
 * Created as network.h on the 2nd of March 2009 from parts of OpenJazz.h
 * Renamed game.h on the 9th of February 2009
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


#ifndef _GAME_H
#define _GAME_H


#include "file.h"
#include "network.h"


// Constants

// Game modes
#define M_SINGLE     0
#define M_COOP       1
#define M_BATTLE     2
#define M_TEAMBATTLE 3

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
#define MT_G_ROAST 0x05 /* Team scored a roast */

#define MT_L_PROP  0x10 /* Level property */
#define MT_L_GRID  0x11 /* Change to gridElement */
#define MT_L_WON   0x12 /* Level over */

#define MT_P_ANIMS 0x20 /* Player animations */
#define MT_P_TEMP  0x21 /* Temporary player properties, e.g. position */

// Minimum message lengths, including header
#define MTL_G_PROPS 8
#define MTL_G_PJOIN 10
#define MTL_G_PQUIT 3
#define MTL_G_LEVEL 4
#define MTL_G_CHECK 4
#define MTL_G_ROAST 3

#define MTL_L_PROP  5
#define MTL_L_GRID  6
#define MTL_L_WON   2

#define MTL_P_ANIMS (PANIMS + 3)
#define MTL_P_TEMP  45

// Array limits
#define MAX_PLAYERS   (MAX_CLIENTS + 1)
#define BUFFER_LENGTH 255 /* Should always be big enough to hold any message */


// Classes

class Game {

	protected:
		char *levelFile;
		int   mode, difficulty;
		int   sendTime, checkTime;

		Game ();

	public:
		Game                   (char *firstLevel, int gameDifficulty);
		virtual ~Game          ();

		int          getMode   ();
		virtual int  setLevel  (char *fileName);
		int          run       ();
		void         view      ();
		virtual void send      (unsigned char *buffer);
		virtual int  playFrame (int ticks);

};


class ServerGame : public Game {

	private:
		int            clientStatus[MAX_CLIENTS]; /*
 			-2: Connected and operational
 			-1: Not connected
			>=0: Number of bytes of the level that have been sent */
		int            clientPlayer[MAX_CLIENTS];
		int            clientSock[MAX_CLIENTS];
		unsigned char  recvBuffers[MAX_CLIENTS][BUFFER_LENGTH];
		int            received[MAX_CLIENTS];
		unsigned char *levelData;
		int            levelSize;
		int            sock;

	public:
		ServerGame     (int gameMode, char *firstLevel, int gameDifficulty);
		~ServerGame    ();

		int  setLevel  (char *fileName);
		void send      (unsigned char *buffer);
		int  playFrame (int ticks);

};


class ClientGame : public Game {

	private:
		File          *file;
		char           ext[3];
		unsigned char  recvBuffer[BUFFER_LENGTH];
		int            received;
		int            clientID;
		int            maxPlayers;
		int            sock;

	public:
		ClientGame     (char *address);
		~ClientGame    ();

		int  setLevel  (char *fileName);
		void send      (unsigned char *buffer);
		int  playFrame (int ticks);

};


// Variable

Extern Game          *game;

#endif

