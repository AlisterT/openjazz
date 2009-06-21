
/*
 *
 * bonus.cpp
 *
 * Created as bonus.c on the 23rd of August 2005
 * Renamed bonus.cpp on the 3rd of February 2009
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

/*
 * Deals with the loading, running and freeing of bonus levels.
 *
 */


#include "bonus.h"
#include "file.h"


Bonus::Bonus (char * fileName) {

	File *file;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	// To do

	delete file;

	return;

}


Bonus::~Bonus () {

	// To do

	return;

}


int Bonus::run () {

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls[C_ESCAPE].state) {

			releaseControl(C_ESCAPE);

			return E_NONE;

		}

		// To do

	}

	return E_NONE;

}


