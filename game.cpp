
/*
 *
 * game.cpp
 *
 * Created on the 9th of March 2009 from parts of menu.cpp and level.cpp
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


#include "font.h"
#include "game.h"
#include "level.h"
#include "menu.h"
#include "network.h"
#include "palette.h"
#include "sound.h"
#include <string.h>


Game::Game () {

	levelFile = NULL;

	players = NULL;

	return;

}


Game::Game (char *firstLevel, int gameDifficulty) {

	levelFile = cloneString(firstLevel);
	mode = M_SINGLE;
	difficulty = gameDifficulty;

	// Create the player
	nPlayers = 1;
	localPlayer = players = new Player[1];
	localPlayer->init(characterName, NULL, 0);

	return;

}


Game::~Game () {

	if (levelFile) delete[] levelFile;

	if (players != NULL) delete[] players;
	localPlayer = NULL;

	return;

}


int Game::getMode () {

	return mode;

}


int Game::setLevel (char *fileName) {

	if (levelFile) delete[] levelFile;

	if (!fileName) levelFile = NULL;
	else levelFile = cloneString(fileName);

	return E_NONE;

}


int Game::run () {

	Scene * scene;
	bool checkpoint;
	int ret;

	checkpoint = false;

	// Play the level(s)
	while (true) {

		sendTime = checkTime = 0;

		// Load the level

		try {

			level = new Level(levelFile, difficulty, checkpoint);

		} catch (int e) {

			return e;

		}

		ret = level->run();

		switch (ret) {

			case E_NONE: // Quit game

				delete level;

				playMusic("menusng.psm");

				return E_NONE;

			case WON: // Completed level

				// If there is no next level, load and run the cutscene
				if (!levelFile) {

					scene = level->createScene();

					delete level;

					scene->run();

					delete scene;

					return E_NONE;

				}

				// Do not use old level's checkpoint coordinates
				checkpoint = false;

				break;

			case LOST: // Lost level

				if (!localPlayer->getLives()) {

					delete level;

					return E_NONE; // Not really a success...

				}

				// Use checkpoint coordinates
				checkpoint = true;

				break;

			default: // Error

				delete level;

				return ret;

		}

		// Unload the previous level
		delete level;

		// Restore menu/level loading screen palette
		usePalette(menu->palettes[1]);

	}

	return E_NONE;

}


void Game::view () {

	// Move the viewport towards the exit sign

	if (checkX << 15 > viewX + (viewW << 9) + (160 * mspf)) viewX += 160 * mspf;
	else if (checkX << 15 < viewX + (viewW << 9) - (160 * mspf))
		viewX -= 160 * mspf;

	if (checkY << 15 > viewY + (viewH << 9) + (160 * mspf)) viewY += 160 * mspf;
	else if (checkY << 15 < viewY + (viewH << 9) - (160 * mspf))
		viewY -= 160 * mspf;

	return;

}


void Game::send (unsigned char *buffer) {

	// Do nothing

	return;

}


int Game::playFrame (int ticks) {

	// Do nothing

	return E_NONE;

}


ServerGame::ServerGame (int gameMode, char *firstLevel, int gameDifficulty) {

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

	mode = gameMode;
	difficulty = gameDifficulty;

	return;

}


ServerGame::~ServerGame () {

	int count;

	for (count = 0; count < MAX_CLIENTS; count++) {

		if (clientStatus[count] != -1) net->close(clientSock[count]);

	}

	net->close(sock);

	if (levelData) delete[] levelData;

	return;

}


int ServerGame::setLevel (char *fileName) {

	File *file;
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

	levelFile = cloneString(fileName);

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


void ServerGame::send (unsigned char *buffer) {

	int count;

	for (count = 0; count < MAX_CLIENTS; count++) {

		// Send data to client, unless the data concerns the client's player
		if ((clientStatus[count] != -1) &&
			(((buffer[1] & MCMASK) != MC_PLAYER) ||
			(buffer[2] != clientPlayer[count])))
			net->send(clientSock[count], buffer);

	}

	return;

}


int ServerGame::playFrame (int ticks) {

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

							printf("Player %d (client %d) joined the game.\n",
								nPlayers, count);


							// Set up the new player

							if (mode == M_COOP) recvBuffers[count][4] = 0;
							else if (mode == M_TEAMBATTLE) {

								// Calculate team imbalance
								length = 0;

								for (pcount = 0; pcount < nPlayers; pcount++) {

									if (players[pcount].getTeam()) length++;
									else length--;

								}

								// Assign to the team with the least players
								if (length >= 0) recvBuffers[count][4] = 0;
								else recvBuffers[count][4] = 1;

							} else {

								recvBuffers[count][4] = 1;

								// Find a team number higher than any other
								for (pcount = nPlayers - 1; pcount >= 0;
									pcount--) {

									if (players[pcount].getTeam() >
										recvBuffers[count][4])
										recvBuffers[count][4] =
											players[pcount].getTeam() + 1;

								}

							}

							players[nPlayers].init((char *)(recvBuffers[count])
								+ 9, recvBuffers[count] + 5,
								recvBuffers[count][4]);
							players[nPlayers].reset();

							printf("Player %d joined team %d.\n",
								nPlayers, recvBuffers[count][4]);

							recvBuffers[count][3] = clientPlayer[count] =
								nPlayers;

							nPlayers++;

						}

						if (recvBuffers[count][1] == MT_G_CHECK) {

							checkX = recvBuffers[count][2];
							checkY = recvBuffers[count][3];

						}

						if (recvBuffers[count][1] == MT_G_ROAST) {

							for (pcount = 0; pcount < nPlayers; pcount++) {

								if (players[pcount].getTeam() ==
									recvBuffers[pcount][2])
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

					printf("Client %d connected.\n", count);

					clientPlayer[count] = -1;
					received[count] = 0;

					// Incorporate the new client

					// Send data
					sendBuffer[0] = MTL_G_PROPS;
					sendBuffer[1] = MT_G_PROPS;
					sendBuffer[2] = 1; // Server version
					sendBuffer[3] = mode;
					sendBuffer[4] = difficulty;
					sendBuffer[5] = MAX_PLAYERS;
					sendBuffer[6] = nPlayers; // Number of players
					sendBuffer[7] = count; // Client's clientID
					net->send(clientSock[count], sendBuffer);

					// Initiate sending of level data
					clientStatus[count] = 0;

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

					printf("Client %d disconnected (code: %d).\n", count,
						net->getError());

					// Disconnect client
					net->close(clientSock[count]);
					clientStatus[count] = -1;

					if (clientPlayer[count] != -1) {

						// Remove the client's player

						printf("Player %d (client %d) left the game.\n",
							clientPlayer[count], count);

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


ClientGame::ClientGame (char *address) {

	unsigned char buffer[BUFFER_LENGTH];
	unsigned int timeout;
	int count, ret;

	sock = net->join(address);

	if (sock < 0) throw sock; // Tee hee hee hee hee.


	// Receive initialisation message

	count = 0;
	timeout = SDL_GetTicks() + T_SCHECK + T_TIMEOUT;

	// Wait for whole message to arrive
	while (count < MTL_G_PROPS) {

		if (loop(NORMAL_LOOP) == E_QUIT) {

			net->close(sock);

			throw E_QUIT;

		}

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			net->close(sock);

			throw E_UNUSED;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);
		fontmn2->showString("WAITING FOR REPLY", screenW >> 2,
			(screenH >> 1) - 16);

		ret = net->recv(sock, buffer + count, MTL_G_PROPS - count);

		if (ret > 0) count += ret;

		if (SDL_GetTicks() > timeout) {

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
	mode = buffer[3];
	difficulty = buffer[4];
	maxPlayers = buffer[5];
	nPlayers = buffer[6];
	clientID = buffer[7];

	if (nPlayers > maxPlayers) {

		net->close(sock);

		throw E_DATA;

	}

	printf("Game mode %d, difficulty %d, %d of %d players.\n", mode,
		difficulty, nPlayers, maxPlayers);

	// Create players
	nPlayers = 0;
	players = new Player[maxPlayers];


	// Download the level from the server

	level = NULL;

	levelFile = cloneString(LEVEL_FILE);
	file = NULL;

	ret = setLevel(NULL);

	if (ret < 0) {

		net->close(sock);

		if (file) delete file;

		throw ret;

	}

	// Add a new player to the game

	buffer[0] = MTL_G_PJOIN + strlen(characterName);
	buffer[1] = MT_G_PJOIN;
	buffer[2] = clientID;
	buffer[3] = 0; // Assigned by the server
	buffer[4] = 0; // Assigned by the server
	memcpy(buffer + 5, characterCols, 4);
	memcpy(buffer + 9, characterName, strlen(characterName) + 1);
	send(buffer);


	// Wait for acknowledgement

	localPlayer = NULL;

	while (!localPlayer) {

		if (loop(NORMAL_LOOP) == E_QUIT) {

			net->close(sock);

			if (file) delete file;

			throw E_QUIT;

		}

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			net->close(sock);

			if (file) delete file;

			throw E_UNUSED;

		}

		clearScreen(0);
		fontmn2->showString("JOINING GAME", screenW >> 2, (screenH >> 1) - 16);

		ret = playFrame(-1);

		if (ret < 0) {

			net->close(sock);

			if (file) delete file;

			throw ret;

		}

	}

	return;

}


ClientGame::~ClientGame () {

	net->close(sock);

	if (file) delete file;

	return;

}


int ClientGame::setLevel (char *fileName) {

	int ret;

	// Free the palette effects
	if (firstPE) {

		delete firstPE;
		firstPE = NULL;

	}

	usePalette(menu->palettes[1]);

	// Wait for level data to start arriving
	while (!file && levelFile) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_UNUSED;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);
		fontmn2->showString("WAITING FOR SERVER", screenW >> 2,
			(screenH >> 1) - 16);

		ret = playFrame(-1);

		if (ret < 0) return ret;

	}

	// Wait for level data to finish arriving
	while (file && levelFile) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_UNUSED;

		}

		SDL_Delay(T_FRAME);

		clearScreen(0);
		fontmn2->showString("downloaded", screenW >> 2, (screenH >> 1) - 16);
		fontmn2->showNumber(file->tell(), (screenW >> 2) + 56, screenH >> 1);
		fontmn2->showString("bytes", (screenW >> 2) + 64, screenH >> 1);

		ret = playFrame(-1);

		if (ret < 0) return ret;

	}

	return E_NONE;

}


void ClientGame::send (unsigned char *buffer) {

	net->send(sock, buffer);

	return;

}


int ClientGame::playFrame (int ticks) {

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

								printf("No level sent.\n");

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

						for (count = nPlayers; count <= recvBuffer[3]; count++)
							{

							players[count].init((char *)recvBuffer + 9,
								recvBuffer + 5, recvBuffer[4]);
							players[count].reset();

							printf("Player %d joined team %d.\n",
								count, recvBuffer[4]);

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

					}

					if (recvBuffer[1] == MT_G_ROAST) {

						for (count = 0; count < nPlayers; count++) {

							if (players[count].getTeam() == recvBuffer[2])
								players[count].teamScore++;

						}

					}

					break;

				case MC_LEVEL:

					if (level) level->receive(recvBuffer);

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

			return E_N_OTHER;

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


