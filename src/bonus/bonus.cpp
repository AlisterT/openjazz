
/*
 *
 * bonus.cpp
 *
 * 23rd August 2005: Created bonus.c
 * 3rd February 2009: Renamed bonus.c to bonus.cpp
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
#include "io/controls.h"
#include "io/file.h"


Bonus::Bonus (char * fileName) {

	File *file;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	// TODO: Load bonus level data

	delete file;

	return;

}


Bonus::~Bonus () {

	// Nothing to do

	return;

}


int Bonus::play () {

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		// TODO: Bonus levels

	}

	return E_NONE;

}


