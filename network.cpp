
/*
 *
 * network.cpp
 *
 * Created on the 3rd of June 2009 from parts of game.cpp
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
#include "network.h"

#ifdef WIN32
	#include <winsock.h>
	#define ioctl ioctlsocket
	#define socklen_t int
	#define EWOULDBLOCK WSAEWOULDBLOCK
	#define MSG_NOSIGNAL 0
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <errno.h>
	#include <string.h>
#endif

Network::Network () {

#ifdef WIN32
	WSADATA WSAData;

	// Start Windows Sockets
	WSAStartup(MAKEWORD(1, 0), &WSAData);
#endif

	return;

}

Network::~Network () {

#ifdef WIN32
	// Shut down Windows Sockets
	WSACleanup();
#endif

	return;

}

int Network::host () {

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

}

int Network::join (char *address) {

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

#ifdef WIN32
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
	timeout = SDL_GetTicks() + T_TIMEOUT;
 
	while (!con) {

		if (loop(NORMAL_LOOP) == E_QUIT) {

			close(sock);

			return E_QUIT;

		}

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			close(sock);

			return E_UNUSED;

		}

		clearScreen(0);
		fontmn2->showString("CONNECTING TO SERVER", screenW >> 2,
			(screenH >> 1) - 16);

		FD_ZERO(&writefds);
		FD_SET(sock, &writefds);
		timeouttv.tv_sec = 0;
		timeouttv.tv_usec = T_FRAME;
		con = select(sock + 1, NULL, &writefds, NULL, &timeouttv);

		if (con == -1) {

			printf("Could not connect to server (code: %d).\n", getError());

			close(sock);

			return E_N_CONNECT;

		}

		if (SDL_GetTicks() > timeout) {

			close(sock);

			return E_TIMEOUT;

		}

	}

	return sock;

}

int Network::accept (int sock) {

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

}

void Network::close (int sock) {

#ifdef WIN32
	closesocket(sock);
#else
	::close(sock);
#endif

	return;

}

int Network::send (int sock, unsigned char *buffer) {

	return ::send(sock, (char *)buffer, buffer[0], MSG_NOSIGNAL);

}

int Network::recv (int sock, unsigned char *buffer, int length) {

	return ::recv(sock, (char *)buffer, length, MSG_NOSIGNAL);

}

bool Network::isConnected (int sock) {

	int length;
	char buffer;

	// Check for incoming data
	length = ::recv(sock, &buffer, 1, MSG_PEEK | MSG_NOSIGNAL);

	// Still connected if data was received or if there was no data to receive
	return (length != -1) || (getError() == EWOULDBLOCK);

}


int Network::getError () {

#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif

}


