
/*
 *
 * servergame.cpp
 *
 * 18th July 2009: Created servergame.cpp from parts of game.cpp
 * 2nd August 2009: Created gamemode.cpp from parts of servergame.cpp
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


#include "game.h"
#include "gamemode.h"

#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/network.h"
#include "level/level.h"
#include "player/player.h"

#include <iostream>
#include <string.h>


ServerGame::ServerGame (GameModeType mode, char* firstLevel, int gameDifficulty) {

	int count;


	// Create the server

	sock = net->host();

	if (sock < 0) throw sock; // Tee hee. Throw sock.


	// Create the players

	nPlayers = 1;
	localPlayer = players = new Player[MAX_PLAYERS];
	localPlayer->init(characterName, characterCols, 0);

	for (count = 0; count < MAX_CLIENTS; count++)
		clientPlayer[count] = clientStatus[count] = -1;


	// Copy the first level into memory

	levelFile = NULL;
	levelData = NULL;

	count = setLevel(firstLevel);

	if (count < 0) {

		net->close(sock);

		if (levelData) delete[] levelData;

		throw count;

	}

	difficulty = gameDifficulty;

	gameMode = createGameMode(mode);

	return;

}


ServerGame::~ServerGame () {

	int count;

	for (count = 0; count < MAX_CLIENTS; count++) {

		if (clientStatus[count] != -1) net->close(clientSock[count]);

	}

	net->close(sock);

	if (levelData) delete[] levelData;

	delete gameMode;

	return;

}


int ServerGame::setLevel (char* fileName) {

	File* file;
	int count;

	if (levelFile) delete[] levelFile;
	if (levelData) delete[] levelData;

	// The new level will be sent to all clients
	for (count = 0; count < MAX_CLIENTS; count++) {

		if (clientStatus[count] != -1) clientStatus[count] = 0;

	}

	if (!fileName) {

		levelFile = NULL;
		levelData = NULL;

		return E_NONE;

	}

	try {

		file = new File(fileName, false);

	} catch (int e) {

		levelFile = NULL;
		levelData = NULL;

		return e;

	}

	levelFile = createString(fileName);

	// Load the entire file into memory
	levelSize = file->getSize();
	levelData = file->loadBlock(levelSize);

	delete file;

	// Modify the extension section to match the actual extension
	count = levelSize - 5;
	while (levelData[count - 1] != 3) count--;
	levelData[count] = fileName[strlen(fileName) - 3];
	levelData[count + 1] = fileName[strlen(fileName) - 2];
	levelData[count + 2] = fileName[strlen(fileName) - 1];

	return E_NONE;

}


void ServerGame::send (unsigned char* buffer) {

	int count;

	for (count = 0; count < MAX_CLIENTS; count++) {

		// Send data to client, unless the data concerns the client's player
		// Each client is solely responsible for its player's state
		if ((clientStatus[count] != -1) &&
			(((buffer[1] & MCMASK) != MC_PLAYER) ||
			(buffer[2] != clientPlayer[count])))
			net->send(clientSock[count], buffer);

	}

	return;

}


int ServerGame::step (unsigned int ticks) {

	unsigned char sendBuffer[BUFFER_LENGTH];
	int count, pcount, length;

	for (count = 0; count < MAX_CLIENTS; count++) {

		if (clientStatus[count] >= 0) {

			// Client is connected, but not operational
			// Send a chunk of the level

			length = levelSize - clientStatus[count];

			if (length > 251) length = 251;

			sendBuffer[0] = MTL_G_LEVEL + length;
			sendBuffer[1] = MT_G_LEVEL;
			sendBuffer[2] = clientStatus[count] >> 8;
			sendBuffer[3] = clientStatus[count] & 255;
			memcpy(sendBuffer + 4, levelData + clientStatus[count], length);
			length = net->send(clientSock[count], sendBuffer);

			// Client is operational if the whole level has been sent
			// Otherwise, keep sending data
			if (length == MTL_G_LEVEL) clientStatus[count] = -2;
			else if (length > 0) clientStatus[count] += length - MTL_G_LEVEL;

		}


		if ((clientStatus[count] == -2) && (received[count] == 0)) {

			// Client is operational, but not currently receiving a message
			// See if there is a new message to receive

			length = net->recv(clientSock[count], recvBuffers[count], 1);

			if (length > 0) received[count]++;

		}


		if ((clientStatus[count] == -2) && (received[count] > 0)) {

			// Currently receiving a message
			// See if there is any more data

			length = net->recv(clientSock[count],
				recvBuffers[count] + received[count],
				recvBuffers[count][0] - received[count]);

			if (length > 0) received[count] += length;


			// See if the whole message has arrived

			if (received[count] >= recvBuffers[count][0]) {

				switch (recvBuffers[count][1] & MCMASK) {

					case MC_GAME:

						if ((recvBuffers[count][1] == MT_G_PJOIN) &&
							(clientPlayer[count] == -1)) {

							std::cout << "Player " << nPlayers << " (client " <<
								count <<") joined the game.\n";


							// Set up the new player

							recvBuffers[count][4] = gameMode->chooseTeam();

							players[nPlayers].init((char *)(recvBuffers[count])
								+ 9, recvBuffers[count] + 5,
								recvBuffers[count][4]);
							players[nPlayers].reset();

							std::cout << "Player " << nPlayers << " joined team " <<
								recvBuffers[count][4] << ".\n";

							recvBuffers[count][3] = clientPlayer[count] =
								nPlayers;

							nPlayers++;

						}

						if (recvBuffers[count][1] == MT_G_CHECK) {

							checkX = recvBuffers[count][2];
							checkY = recvBuffers[count][3];

						}

						if (recvBuffers[count][1] == MT_G_SCORE) {

							for (pcount = 0; pcount < nPlayers; pcount++) {

								if (players[pcount].getTeam() ==
									recvBuffers[count][2])
									players[pcount].teamScore++;

							}

						}

						break;

					case MC_LEVEL:

						level->receive(recvBuffers[count]);

						break;

					case MC_PLAYER:

						// Assign player byte based on sender
						recvBuffers[count][2] = clientPlayer[count];

						players[clientPlayer[count]].
							receive(recvBuffers[count]);

						break;

				}

				// Update clients
				send(recvBuffers[count]);

				received[count] = 0;

			}

		}

		if (ticks >= checkTime) {

			if ((clientStatus[count] == -1) && levelData) {

				// Client is not connected
				// Check for new connection

				clientSock[count] = net->accept(sock);

				if (clientSock[count] != -1) {

					std::cout << "Client " << count << " connected.\n";

					clientPlayer[count] = -1;
					received[count] = 0;

					// Incorporate the new client

					// Send data
					sendBuffer[0] = MTL_G_PROPS;
					sendBuffer[1] = MT_G_PROPS;
					sendBuffer[2] = 1; // Server version
					sendBuffer[3] = gameMode->getMode();
					sendBuffer[4] = difficulty;
					sendBuffer[5] = MAX_PLAYERS;
					sendBuffer[6] = nPlayers; // Number of players
					sendBuffer[7] = count; // Client's clientID
					net->send(clientSock[count], sendBuffer);

					// Initiate sending of level data
					clientStatus[count] = 0;

					// Inform the new client of the checkpoint
					sendBuffer[0] = MTL_G_CHECK;
					sendBuffer[1] = MT_G_CHECK;
					sendBuffer[2] = checkX;
					sendBuffer[3] = checkY;
					net->send(clientSock[count], sendBuffer);

					// Inform the new client of the existing players

					sendBuffer[1] = MT_G_PJOIN;

					for (pcount = 0; pcount < nPlayers; pcount++) {

						sendBuffer[0] = MTL_G_PJOIN +
							strlen(players[pcount].getName());
						sendBuffer[2] = count;
						sendBuffer[3] = pcount;
						sendBuffer[4] = players[pcount].getTeam();
						memcpy(sendBuffer + 5, players[pcount].getCols(), 4);
						memcpy(sendBuffer + 9, players[pcount].getName(),
							strlen(players[pcount].getName()) + 1);
						net->send(clientSock[count], sendBuffer);

					}

				}

			} else {

				// Client is connected
				// Check for disconnection

				if (!(net->isConnected(clientSock[count]))) {

					std::cout << "Client " << count << " disconnected (code: " <<
						net->getError() << ").\n";

					// Disconnect client
					net->close(clientSock[count]);
					clientStatus[count] = -1;

					if (clientPlayer[count] != -1) {

						// Remove the client's player

						std::cout << "Player " << clientPlayer[count] <<
							" (client " << count << ") left the game.\n";

						nPlayers--;

						players[clientPlayer[count]].deinit();

						// If necessary, move more recent players
						for (pcount = clientPlayer[count]; pcount < nPlayers;
							pcount++)
							memcpy(players + pcount, players + pcount + 1,
								sizeof(Player));

						// Clear duplicate pointers
						memset(players + nPlayers, 0, sizeof(Player));

						// Inform remaining clients that the player has left
						sendBuffer[0] = MTL_G_PQUIT;
						sendBuffer[1] = MT_G_PQUIT;
						sendBuffer[2] = clientPlayer[count];
						send(sendBuffer);

						clientPlayer[count] = -1;

					}

				}

			}

		}

	}

	if (ticks >= checkTime) checkTime = ticks + T_SCHECK;

	if (ticks >= sendTime) {

		// Update clients

		sendBuffer[0] = MTL_P_TEMP;
		sendBuffer[1] = MT_P_TEMP;

		for (count = 0; count < nPlayers; count++) {

			sendBuffer[2] = count;
			players[count].send(sendBuffer);
			send(sendBuffer);

		}

		sendTime = ticks + T_SSEND;

	}

	return E_NONE;

}


void ServerGame::score (unsigned char team) {

	unsigned char buffer[MTL_G_SCORE];
	int count;

	// Inform clients
	buffer[0] = MTL_G_SCORE;
	buffer[1] = MT_G_SCORE;
	buffer[2] = team;
	send(buffer);

	// Update self
	for (count = 0; count < nPlayers; count++) {

		if (players[count].getTeam() == team) players[count].teamScore++;

	}

	return;

}


void ServerGame::setCheckpoint (unsigned char gridX, unsigned char gridY) {

	unsigned char buffer[MTL_G_CHECK];

	buffer[0] = MTL_G_CHECK;
	buffer[1] = MT_G_CHECK;
	buffer[2] = gridX;
	buffer[3] = gridY;
	send(buffer);

	checkX = gridX;
	checkY = gridY;

	return;

}


