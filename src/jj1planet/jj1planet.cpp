
/**
 *
 * @file jj1planet.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created planet.c
 * 3rd February 2009: Renamed planet.c to planet.cpp
 * 1st August 2012: Renamed planet.cpp to jj1planet.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the loading, displaying and freeing of the planet landing
 * sequence.
 *
 */


#include "jj1planet.h"

#include "io/controls.h"
#include "io/file.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "loop.h"
#include "util.h"

#include <string.h>


/**
 * Create a JJ1 planet approach sequence.
 *
 * @param fileName Name of the file containing the planet data
 * @param previous The ID of the last planet approach sequence
 */
JJ1Planet::JJ1Planet (char * fileName, int previous) {

	File *file;
	unsigned char *pixels;
	int count;

	try {

		file = new File(fileName, false);

	} catch (int e) {

		throw e;

	}

	id = file->loadShort();

	if (id == previous) {

		// Not approaching a planet if already there

		delete file;

		throw E_NONE;

	}

	// Load planet name
	name = file->loadString();

	// Lower-case the name
	for (count = 0; name[count]; count++) {

		if ((name[count] >= 65) && (name[count] <= 90)) name[count] += 32;

	}

	// Load the palette
	file->loadPalette(palette, false);

	// Load the planet image
	pixels = file->loadBlock(64 * 55);
	sprite.setPixels(pixels, 64, 55, 0);
	delete[] pixels;


	delete file;

	return;

}


/**
 * Delete the JJ1 planet approach sequence.
 */
JJ1Planet::~JJ1Planet () {

	delete[] name;

	return;

}


/**
 * Get the ID of the planet approach squence.
 *
 * @return The ID
 */
int JJ1Planet::getId () {

	return id;

}


/**
 * Run the JJ1 planet approach sequence.
 *
 * @return Error code
 */
int JJ1Planet::play () {

	unsigned int tickOffset;

	tickOffset = globalTicks;

	stopMusic();

	video.setPalette(palette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE) || controls.wasCursorReleased()) return E_NONE;

		SDL_Delay(T_FRAME);

		video.clearScreen(0);

		if (globalTicks - tickOffset < F2)
			sprite.drawScaled(canvasW >> 1, canvasH >> 1, globalTicks - tickOffset);
		else if (globalTicks - tickOffset < F4)
			sprite.drawScaled(canvasW >> 1, canvasH >> 1, F2);
		else if (globalTicks - tickOffset < F4 + FQ)
			sprite.drawScaled(canvasW >> 1, canvasH >> 1, (globalTicks - tickOffset - F4) * 32 + F2);
		else return E_NONE;

		fontmn1->showString("now approaching", (canvasW - 288) >> 1, 0);
		fontmn1->showString(name, (canvasW - fontmn1->getStringWidth(name)) >> 1, canvasH - 24);

	}

	return E_NONE;

}


