
/**
 *
 * @file network.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 3rd June 2009: Created network.h from parts of OpenJazz.h
 *
 * @section Licence
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

#ifndef _NETWORK_H
#define _NETWORK_H


#include "OpenJazz.h"

#ifdef USE_SDL_NET
#include <SDL_net.h>
#endif

// Constants

// Defaults
#if defined(DINGOO)
	#define NET_ADDRESS "10.1.0.1"
#else
	#define NET_ADDRESS "192.168.0.1"
#endif
#define NET_PORT    10052

// Timeout interval
#define T_TIMEOUT 30000

// Client limit
#define MAX_CLIENTS   31


// Class

/// Networking
class Network {

	public:
#ifdef USE_SDL_NET
		TCPsocket socket;
		TCPsocket clientSocket;
		IPaddress ipAddress;
		SDLNet_SocketSet socketset;
#endif

		Network          ();
		~Network         ();

		int  host        ();
		int  join        (char *address);
		int  accept      (int sock);
		void close       (int sock);
		int  send        (int sock, unsigned char *buffer);
		int  recv        (int sock, unsigned char *buffer, int length);
		bool isConnected (int sock);
		int  getError    ();

};


// Variables

EXTERN char    *netAddress; /// Server address
EXTERN Network *net;

#endif

