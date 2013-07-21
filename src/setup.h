
/**
 *
 * @file setup.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 3rd February 2009: Created menu.h from parts of OpenJazz.h
 * 21st July 2013: Created setup.h from parts of menu.h
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


#ifndef _SETUP_H
#define _SETUP_H


#include "player/player.h"

#include "OpenJazz.h"


// Class

/// Configuration
class Setup {

	public:
		char*         characterName;
		unsigned char characterCols[PCOLOURS];
		bool          slowMotion;
		bool          leaveUnneeded;
		bool          manyBirds;

		Setup  ();
		~Setup ();

		void load (int* videoW, int* videoH, int* videoScale);
		void save ();

};


// Variable

EXTERN Setup setup;

#endif

