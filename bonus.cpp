
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


#include "OpenJazz.h"


Bonus::Bonus (char * fn) {

	File *f;

	try {

		f = new File(fn, false);

	} catch (int e) {

		throw FAILURE;

	}

	// To do

	delete f;

	return;

}


Bonus::~Bonus () {

	// To do

	return;

}


int Bonus::run () {

	while (1) {

		if (loop() == QUIT) return QUIT;

		if (controls[C_ESCAPE].state == SDL_PRESSED) {

			releaseControl(C_ESCAPE);

			return SUCCESS;

		}

		// To do

	}

	return SUCCESS;

}


