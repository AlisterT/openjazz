
/*
 *
 * network.h
 *
 * Created on the 3rd of June 2009 from parts of OpenJazz.h
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

#ifndef _NETWORK_H
#define _NETWORK_H


#include "OpenJazz.h"


// Constants

// Defaults
#define NET_ADDRESS "192.168.0.1"
#define NET_PORT    10052

// Timeout interval
#define T_TIMEOUT 30000

// Client limit
#define MAX_CLIENTS   31


// Class

class Network {

	public:
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

Extern Network *net;

#endif

