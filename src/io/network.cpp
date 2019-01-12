
/**
 *
 * @file network.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c and menu.c
 * - 3rd of February 2009: Renamed level.c to level.cpp and menu.c to menu.cpp
 * - 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * - 3rd June 2009: Created network.cpp from parts of game.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with a platform-specific networking API.
 *
 * On most platforms, USE_SOCKETS should be defined.
 *
 */


#include "controls.h"
#include "gfx/font.h"
#include "gfx/video.h"
#include "network.h"

#include "loop.h"
#include "util.h"

#ifdef USE_SOCKETS
	#ifdef _WIN32
		#include <winsock.h>
		#define ioctl ioctlsocket
		#define socklen_t int
		#define EWOULDBLOCK WSAEWOULDBLOCK
		#define MSG_NOSIGNAL 0
	#else
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <arpa/inet.h>
		#include <sys/select.h>
		#include <sys/ioctl.h>
		#include <netinet/in.h>
		#include <unistd.h>
		#include <errno.h>
		#include <string.h>
	#endif
	#ifdef __APPLE__
		#define MSG_NOSIGNAL 0
	#endif
#elif defined(WII)
	#include <network.h>
#elif defined(USE_SDL_NET)
	#include <arpa/inet.h>
#endif


/**
 * Initialise networking.
 */
Network::Network () {

#ifdef USE_SOCKETS
	#ifdef _WIN32
	WSADATA WSAData;

	// Start Windows Sockets
	WSAStartup(MAKEWORD(1, 0), &WSAData);
	#endif
#elif defined USE_SDL_NET
#  ifdef WII
	char ip[16];

	// Initialize Wii networking (using dhcp)
	if_config(ip, NULL, NULL, true, 20);
#  endif
	SDLNet_Init();
#endif

	return;

}


/**
 * De-initialise networking.
 */
Network::~Network () {

#ifdef USE_SOCKETS
	#ifdef _WIN32
	// Shut down Windows Sockets
	WSACleanup();
	#endif
#elif defined USE_SDL_NET
	SDLNet_Quit();
#  ifdef WII
	net_deinit();
#  endif
#endif

	return;

}


/**
 * Open a host connection.
 *
 * @return Connection socket or error code
 */
int Network::host () {

#ifdef USE_SOCKETS
	sockaddr_in sockAddr;
	int sock, nonblock;


	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) return E_N_SOCKET;


	// Make the socket non-blocking
	nonblock = 1;
	ioctl(sock, FIONBIO, (u_long *)&nonblock);

	memset(&sockAddr, 0, sizeof(sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;
	sockAddr.sin_port = htons(NET_PORT);


	if (bind(sock, (sockaddr *)&sockAddr, sizeof(sockaddr_in))) {

		close(sock);

		return E_N_BIND;

	}

	if (listen(sock, MAX_CLIENTS) == -1) {

		close(sock);

		return E_N_LISTEN;

	}

	return sock;
#elif defined USE_SDL_NET
	ipAddress.port = NET_PORT;
	ipAddress.host = 0;
	socket = SDLNet_TCP_Open(&ipAddress);

	if (socket == NULL) return E_N_SOCKET;

	return (int)socket;
#else
	return E_N_OTHER;
#endif

}


/**
 * Open a client connection to the specified server.
 *
 * @param address Address of the server
 *
 * @return Connection socket or error code
 */
int Network::join (char *address) {

#ifdef USE_SOCKETS
	sockaddr_in sockAddr;
	fd_set writefds;
	timeval timeouttv;
	unsigned int timeout;
	int sock, con;

	// Create socket

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) return E_N_SOCKET;

	// Make socket non-blocking
	con = 1;
	ioctl(sock, FIONBIO, (u_long *)&con);


	// Connect to server

	memset(&sockAddr, 0, sizeof(sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(NET_PORT);

	#ifdef _WIN32
	sockAddr.sin_addr.s_addr = inet_addr(address);
	#else
	if (inet_aton(address, &(sockAddr.sin_addr)) == 0) return E_N_ADDRESS;
	#endif

	// Initiate connection
	con = connect(sock, (sockaddr *)&sockAddr, sizeof(sockAddr));

	// If the connection completed, return
	if (con == 0) return sock;


	// Wait for connection to complete

	con = 0;
	timeout = globalTicks + T_TIMEOUT;

	while (!con) {

		if (loop(NORMAL_LOOP) == E_QUIT) {

			close(sock);

			return E_QUIT;

		}

		if (controls.release(C_ESCAPE)) {

			close(sock);

			return E_RETURN;

		}

		video.clearScreen(0);
		fontmn2->showString("CONNECTING TO SERVER", canvasW >> 2, (canvasH >> 1) - 16);

		FD_ZERO(&writefds);
		FD_SET(sock, &writefds);
		timeouttv.tv_sec = 0;
		timeouttv.tv_usec = T_MENU_FRAME;
		con = select(sock + 1, NULL, &writefds, NULL, &timeouttv);

		if (con == -1) {

			log("Could not connect to server - code", getError());

			close(sock);

			return E_N_CONNECT;

		}

		if (globalTicks > timeout) {

			close(sock);

			return E_TIMEOUT;

		}

	}

	return sock;
#elif defined USE_SDL_NET
	video.clearScreen(0);
	fontmn2->showString("CONNECTING TO SERVER", canvasW >> 2, (canvasH >> 1) - 16);
	loop(NORMAL_LOOP);
	ipAddress.port = NET_PORT;
	ipAddress.host = inet_addr(address);
	socket = SDLNet_TCP_Open(&ipAddress);

	if (socket == NULL) return -1;

	return (int)socket;
#else
	return E_N_OTHER;
#endif

}


/**
 * Accept a connection to a client
 *
 * @param sock The host connection socket
 *
 * @return Client connection socket, or -1 for no connection
 */
int Network::accept (int sock) {

#ifdef USE_SOCKETS
	sockaddr_in sockAddr;
	int clientSocket, length;

	length = sizeof(sockaddr_in);

	clientSocket = ::accept(sock, (sockaddr *)&sockAddr, (socklen_t *)&length);

	if (clientSocket != -1) {

		// Make the socket non-blocking
		length = 1;
		ioctl(clientSocket, FIONBIO, (u_long *)&length);

	}

	return clientSocket;
#elif defined USE_SDL_NET
	clientSocket = SDLNet_TCP_Accept((TCPsocket)sock);

	if (clientSocket == NULL) return -1;

	return (int)&clientSocket;
#else
	return -1;
#endif

}


/**
 * Close a connection.
 *
 * @param sock The connection socket
 */
void Network::close (int sock) {

#ifdef USE_SOCKETS
	#ifdef _WIN32
	closesocket(sock);
	#else
	::close(sock);
	#endif
#elif defined USE_SDL_NET
	SDLNet_TCP_Close((TCPsocket)sock);
#endif

	return;

}


/**
 * Send data over the specified connection.
 *
 * @param sock Connection socket
 * @param buffer Data to be sent
 *
 * @return Number of bytes sent, or -1 for failure
 */
int Network::send (int sock, unsigned char *buffer) {

#ifdef USE_SOCKETS
	return ::send(sock, (char *)buffer, buffer[0], MSG_NOSIGNAL);
#elif defined USE_SDL_NET
	return SDLNet_TCP_Send((TCPsocket)sock, (char *)buffer, buffer[0]);
#else
	return 0;
#endif

}


/**
 * Receive data from the specified connection.
 *
 * @param sock Connection socket
 * @param buffer Buffer to receive data
 * @param length The size of the buffer, in bytes
 *
 * @return Number of bytes received, or -1 for failure
 */
int Network::recv (int sock, unsigned char *buffer, int length) {

#ifdef USE_SOCKETS
	return ::recv(sock, (char *)buffer, length, MSG_NOSIGNAL);
#elif defined USE_SDL_NET
	return SDLNet_TCP_Recv((TCPsocket)sock, buffer, length);
#else
	return 0;
#endif

}


/**
 * Check if a given socket is connected.
 *
 * @param sock The socket
 *
 * @return True if connected
 */
bool Network::isConnected (int sock) {

#ifdef USE_SOCKETS
	int length;
	char buffer;

	// Check for incoming data
	length = ::recv(sock, &buffer, 1, MSG_PEEK | MSG_NOSIGNAL);

	// Still connected if data was received or if there was no data to receive
	return (length != -1) || (getError() == EWOULDBLOCK);
#elif defined USE_SDL_NET
	return SDLNet_SocketReady((TCPsocket)sock);
#else
	return false;
#endif

}


/**
 * Get the last error code.
 *
 * @return Network library error code
 */
int Network::getError () {

#ifdef USE_SOCKETS
	#ifdef _WIN32
	return WSAGetLastError();
	#else
	return errno;
	#endif
#elif defined USE_SDL_NET
	return (int)SDLNet_GetError();
#else
	return 0;
#endif

}


