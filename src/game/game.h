
/*
 *
 * game.h
 *
 * 2nd March 2009: Created network.h from parts of OpenJazz.h
 * 9th February 2009: Renamed network.h to game.h
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


#ifndef _GAME_H
#define _GAME_H


#include "gamemode.h"

#include "io/network.h"


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

#define MT_L_PROP  0x10 /* Level property */
#define MT_L_GRID  0x11 /* Change to gridElement */
#define MT_L_STAGE 0x12 /* Change in level stage */

#define MT_P_ANIMS 0x20 /* Player animations */
#define MT_P_TEMP  0x21 /* Temporary player properties, e.g. position */

// Minimum message lengths, including header
#define MTL_G_PROPS 8
#define MTL_G_PJOIN 10
#define MTL_G_PQUIT 3
#define MTL_G_LEVEL 4
#define MTL_G_CHECK 4
#define MTL_G_SCORE 3

#define MTL_L_PROP  5
#define MTL_L_GRID  6
#define MTL_L_STAGE 3

#define MTL_P_ANIMS (PANIMS + 3)
#define MTL_P_TEMP  46

#define BUFFER_LENGTH 255 /* Should always be big enough to hold any message */


// Classes

class File;
class Player;

class Game {

	protected:
		char          *levelFile;
		int            difficulty;
		unsigned int   sendTime, checkTime;
		unsigned char  checkX, checkY;

		Game ();

	public:
		Game                       (char *firstLevel, int gameDifficulty);
		virtual ~Game              ();

		virtual int  setLevel      (char *fileName);
		int          play          ();
		void         view          (int change);
		virtual void send          (unsigned char *buffer);
		virtual int  step          (unsigned int ticks);
		virtual void score         (unsigned char team);
		virtual void setCheckpoint (unsigned char gridX, unsigned char gridY);
		void         resetPlayer   (Player *player, bool bonus);

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
		ServerGame         (GameModeType mode, char *firstLevel, int gameDifficulty);
		~ServerGame        ();

		int  setLevel      (char *fileName);
		void send          (unsigned char *buffer);
		int  step          (unsigned int ticks);
		void score         (unsigned char team);
		void setCheckpoint (unsigned char gridX, unsigned char gridY);

};


class ClientGame : public Game {

	private:
		File          *file;
		unsigned char  recvBuffer[BUFFER_LENGTH];
		int            received;
		int            clientID;
		int            maxPlayers;
		int            sock;

	public:
		ClientGame         (char *address);
		~ClientGame        ();

		int  setLevel      (char *fileName);
		void send          (unsigned char *buffer);
		int  step          (unsigned int ticks);
		void score         (unsigned char team);
		void setCheckpoint (unsigned char gridX, unsigned char gridY);

};


// Variable

EXTERN Game *game;

#endif

