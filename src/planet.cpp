
/*
 *
 * planet.cpp
 *
 * 23rd August 2005: Created planet.c
 * 3rd February 2009: Renamed planet.c to planet.cpp
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
 * Deals with the loading, displaying and freeing of the planet landing
 * sequence.
 *
 */


#include "planet.h"

#include "io/controls.h"
#include "io/file.h"


Planet::Planet (char * fileName) {

	File *file;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	// TODO: Load planet file

	delete file;

	return;

}


Planet::~Planet () {

	// Nothing to do

	return;

}


int Planet::play () {

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_NONE;

		// TODO: Display planet

	}

	return E_NONE;

}


