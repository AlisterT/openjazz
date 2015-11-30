
/**
 *
 * @file clientgame.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created level.c and menu.c
 * 3rd of February 2009: Renamed level.c to level.cpp and menu.c to menu.cpp
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 18th July 2009: Created clientgame.cpp from parts of game.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "game.h"
#include "gamemode.h"

#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/network.h"
#include "player/player.h"
#include "loop.h"
#include "setup.h"
#include "util.h"

#include <string.h>


/**
 * Create game client
 *
 * @param address Address of the server to which to connect
 */
ClientGame::ClientGame (char* address) {

	unsigned char buffer[BUFFER_LENGTH];
	unsigned int timeout;
	int count, ret;
	GameModeType modeType;

	sock = net->join(address);

	if (sock < 0) throw sock; // Tee hee hee hee hee.


	// Receive initialisation message

	count = 0;
	timeout = globalTicks + T_SCHECK + T_TIMEOUT;

	// Wait for whole message to arrive
	while (count < MTL_G_PROPS) {

		if (loop(NORMAL_LOOP) == E_QUIT) {

			net->close(sock);

			throw E_QUIT;

		}

		if (controls.release(C_ESCAPE)) {

			net->close(sock);

			throw E_RETURN;

		}

		SDL_Delay(T_FRAME);

		video.clearScreen(0);
		fontmn2->showString("WAITING FOR REPLY", canvasW >> 2, (canvasH >> 1) - 16);

		ret = net->recv(sock, buffer + count, MTL_G_PROPS - count);

		if (ret > 0) count += ret;

		if (globalTicks > timeout) {

			net->close(sock);

			throw E_TIMEOUT;

		}

	}

	// Make sure message is valid
	if (buffer[1] != MT_G_PROPS) {

		net->close(sock);

		throw E_DATA;

	} else if (buffer[2] != 1) {

		net->close(sock);

		throw E_VERSION;

	}

	printf("Connected to server (version %d).\n", buffer[2]);

	// Copy game parameters
	modeType = GameModeType(buffer[3]);
	difficulty = buffer[4];
	maxPlayers = buffer[5];
	nPlayers = buffer[6];
	clientID = buffer[7];

	printf("Game mode %d, difficulty %d, %d of %d players.\n", modeType, difficulty, nPlayers, maxPlayers);

	if (nPlayers > maxPlayers) {

		net->close(sock);

		throw E_DATA;

	}


	mode = createMode(modeType);

	if (!mode) {

		net->close(sock);

		throw E_DATA;

	}


	// Create players
	nPlayers = 0;
	players = new Player[maxPlayers];


	// Download the level from the server

	levelFile = createString(LEVEL_FILE);
	file = NULL;

	ret = setLevel(NULL);

	if (ret < 0) {

		net->close(sock);

		if (file) delete file;

		delete mode;

		throw ret;

	}

	// Add a new player to the game

	buffer[0] = MTL_G_PJOIN + strlen(setup.characterName);
	buffer[1] = MT_G_PJOIN;
	buffer[2] = clientID;
	buffer[3] = 0; // Player's number, assigned by the server
	buffer[4] = 0; // Player's team, assigned by the server
	memcpy(buffer + 5, setup.characterCols, 4);
	memcpy(buffer + 9, setup.characterName, strlen(setup.characterName) + 1);
	send(buffer);


	// Wait for acknowledgement

	localPlayer = NULL;

	while (!localPlayer) {

		if (loop(NORMAL_LOOP) == E_QUIT) {

			net->close(sock);

			if (file) delete file;

			delete mode;

			throw E_QUIT;

		}

		if (controls.release(C_ESCAPE)) {

			net->close(sock);

			if (file) delete file;

			delete mode;

			throw E_RETURN;

		}

		video.clearScreen(0);
		fontmn2->showString("JOINING GAME", canvasW >> 2, (canvasH >> 1) - 16);

		ret = step(0);

		if (ret < 0) {

			net->close(sock);

			if (file) delete file;

			delete mode;

			throw ret;

		}

	}

	return;

}


/**
 * Disconnect and destroy client
 */
ClientGame::~ClientGame () {

	net->close(sock);

	if (file) delete file;

	delete mode;

	return;

}


/**
 * Set the next level and receive level data from server
 *
 * @param fileName The file name of the next level
 *
 * @return Error code
 */
int ClientGame::setLevel (char* fileName) {

	(void)fileName;

	int ret;

	video.setPalette(menuPalette);

	// Wait for level data to start arriving
	while (!file && levelFile) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_RETURN;

		SDL_Delay(T_FRAME);

		video.clearScreen(0);
		fontmn2->showString("WAITING FOR SERVER", canvasW >> 2, (canvasH >> 1) - 16);

		ret = step(0);

		if (ret < 0) return ret;

	}

	// Wait for level data to finish arriving
	while (file && levelFile) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_RETURN;

		SDL_Delay(T_FRAME);

		video.clearScreen(0);
		fontmn2->showString("downloaded", canvasW >> 2, (canvasH >> 1) - 16);
		fontmn2->showNumber(file->tell(), (canvasW >> 2) + 56, canvasH >> 1);
		fontmn2->showString("bytes", (canvasW >> 2) + 64, canvasH >> 1);

		ret = step(0);

		if (ret < 0) return ret;

	}

	return E_NONE;

}


/**
 * Send data to server
 *
 * @param buffer Data to send. First byte indicates length.
 */
void ClientGame::send (unsigned char* buffer) {

	net->send(sock, buffer);

	return;

}


/**
 * Game iteration
 *
 * @param ticks Current time
 *
 * @return Error code
 */
int ClientGame::step (unsigned int ticks) {

	unsigned char sendBuffer[BUFFER_LENGTH];
	int length, count;
	bool firstMessage;

	// Receive data from server

	if (received == 0) {

		// Not currently receiving a message
		// See if there is a new message to receive

		length = net->recv(sock, recvBuffer, 1);

		if (length > 0) received++;

	}

	if (received > 0) {

		// Currently receiving a message
		// See if there is any more data

		length = net->recv(sock, recvBuffer + received,
			recvBuffer[0] - received);

		if (length > 0) received += length;


		// See if the whole message has arrived

		if (received >= recvBuffer[0]) {

			switch (recvBuffer[1] & MCMASK) {

				case MC_GAME:

					if (recvBuffer[1] == MT_G_LEVEL) {

						if (!file) {

							// Not already storing level data, so open the file

							try {

								file = new File(levelFile, true);

							} catch (int e) {

								return e;

							}

							firstMessage = true;

						} else firstMessage = false;

						file->seek((recvBuffer[2] << 8) + recvBuffer[3], true);

						for (count = 4; count < recvBuffer[0]; count++)
							file->storeChar(recvBuffer[count]);

						// If a zero-length block has been sent, it is the last
						if (recvBuffer[0] == MTL_G_LEVEL) {

							if (firstMessage) {

								// If the last message was also the first,
								// then the run of levels has ended

								delete[] levelFile;
								levelFile = NULL;

							}

							delete file;
							file = NULL;

						}

						break;

					}

					if ((recvBuffer[1] == MT_G_PJOIN) &&
						(recvBuffer[3] < maxPlayers)) {

						printf("Player %d joined the game.\n", recvBuffer[3]);

						// Add the new player, and any that have been missed

						for (count = nPlayers; count <= recvBuffer[3]; count++) {

							players[count].init(this, (char *)recvBuffer + 9,
								recvBuffer + 5, recvBuffer[4]);
							addLevelPlayer(players + count);

							printf("Player %d joined team %d.\n", count, recvBuffer[4]);

						}

						nPlayers = count;

						if (recvBuffer[2] == clientID)
							localPlayer = players + recvBuffer[3];

					}

					if ((recvBuffer[1] == MT_G_PQUIT) &&
						(recvBuffer[2] < nPlayers)) {

						printf("Player %d left the game.\n", recvBuffer[2]);

						// Remove the player

						players[recvBuffer[2]].deinit();

						// If necessary, move more recent players
						for (count = recvBuffer[2]; count < nPlayers; count++)
							memcpy(players + count, players + count + 1,
								sizeof(Player));

						// Clear duplicate pointers
						memset(players + nPlayers, 0, sizeof(Player));

					}

					if (recvBuffer[1] == MT_G_CHECK) {

						checkX = recvBuffer[2];
						checkY = recvBuffer[3];

						if (recvBuffer[0] > 4) {

							checkX += recvBuffer[4] << 8;
							checkY += recvBuffer[5] << 8;

						}

					}

					if (recvBuffer[1] == MT_G_SCORE) {

						for (count = 0; count < nPlayers; count++) {

							if (players[count].getTeam() == recvBuffer[2])
								players[count].teamScore++;

						}

					}

					if (recvBuffer[1] == MT_G_LTYPE) {

						levelType = (LevelType)recvBuffer[2];

					}

					break;

				case MC_LEVEL:

					if (baseLevel) baseLevel->receive(recvBuffer);

					break;

				case MC_PLAYER:

					if (recvBuffer[2] < maxPlayers)
						players[recvBuffer[2]].receive(recvBuffer);

					break;

			}

			received = 0;

		}

	}

	if (ticks >= checkTime) {

		// Check for disconnection

		if (!(net->isConnected(sock))) {

			if (file) delete file;
			file = NULL;

			return E_N_DISCONNECT;

		}

		checkTime = ticks + T_CCHECK;

	}

	if (localPlayer && (ticks >= sendTime)) {

		// Update server

		sendBuffer[0] = MTL_P_TEMP;
		sendBuffer[1] = MT_P_TEMP;
		sendBuffer[2] = 0;
		localPlayer->send(sendBuffer);
		send(sendBuffer);

		sendTime = ticks + T_CSEND;

	}

	return E_NONE;

}


/**
 * Ask server to award team a point
 *
 * @param team Team to receive point
 */
void ClientGame::score (unsigned char team) {

	unsigned char buffer[MTL_G_SCORE];

	// Inform server
	buffer[0] = MTL_G_SCORE;
	buffer[1] = MT_G_SCORE;
	buffer[2] = team;
	send(buffer);

	return;

}


/**
 * Ask server to approve new checkpoint
 *
 * @param gridX X-coordinate (in tiles) of the checkpoint
 * @param gridY Y-coordinate (in tiles) of the checkpoint
 */
void ClientGame::setCheckpoint (int gridX, int gridY) {

	unsigned char buffer[MTL_G_CHECK];

	buffer[0] = MTL_G_CHECK;
	buffer[1] = MT_G_CHECK;
	buffer[2] = gridX & 0xFF;
	buffer[3] = gridY & 0xFF;
	buffer[4] = (gridX >> 8) & 0xFF;
	buffer[5] = (gridY >> 8) & 0xFF;
	send(buffer);

	return;

}


