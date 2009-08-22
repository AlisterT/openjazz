
/*
 *
 * menu.cpp
 *
 * 23rd of August 2005: Created menu.c
 * 3rd of February 2009: Renamed menu.c to menu.cpp
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 18th July 2009: Created menugame.cpp from parts of menu.cpp
 * 18th July 2009: Created menuutil.cpp from parts of menu.cpp
 * 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * 19th July 2009: Created menumain.cpp from parts of menu.cpp
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
 * Deals with the loading and freeing of the menu data.
 *
 */


#include "menu.h"

#include "game/game.h"
#include "io/file.h"
#include "io/gfx/video.h"
#include "io/sound.h"

#include <time.h>


Menu::Menu () {

	File *file;
	unsigned char *pixels;
	time_t currentTime;
	int count, col;


	// Load the OpenJazz logo

	try {

		file = new File(LOGO_FILE, false);

	} catch (int e) {

		throw e;

	}

	screens[14] = file->loadSurface(64, 40);

	delete file;


	// Load the menu graphics

	try {

		file = new File(F_MENU, false);

	} catch (int e) {

		SDL_FreeSurface(screens[14]);

		throw e;

	}

	file->seek(0, true);

	// Load the main menu graphics
	file->loadPalette(palettes[0]);
	screens[0] = file->loadSurface(320, 200);
	screens[1] = file->loadSurface(320, 200);


	if (file->getSize() > 200000) {

		time(&currentTime);

		// In December, load the Christmas menu graphics
		if (localtime(&currentTime)->tm_mon == 11) {

			SDL_FreeSurface(screens[0]);
			SDL_FreeSurface(screens[1]);
			file->loadPalette(palettes[0]);
			screens[0] = file->loadSurface(320, 200);
			screens[1] = file->loadSurface(320, 200);

		} else {

			file->skipRLE();
			file->skipRLE();
			file->skipRLE();

		}

	}

	SDL_SetColorKey(screens[0], SDL_SRCCOLORKEY, 0);
	SDL_SetColorKey(screens[1], SDL_SRCCOLORKEY, 0);


	// Load the difficulty graphics
	file->loadPalette(palettes[1]);
	screens[2] = file->loadSurface(320, 200);
	SDL_SetColorKey(screens[2], SDL_SRCCOLORKEY, 0);

	// Default difficulty setting
	difficulty = 1;


	// Load the episode pictures (max. 10 episodes + bonus level)

	// Load their palette
	file->loadPalette(palettes[2]);

	// Generate a greyscale mapping
	for (count = 0; count < 256; count++) {

		col = ((palettes[2][count].r >> 1) + (palettes[2][count].g << 1) +
			(palettes[2][count].b >> 1)) >> 3;

		if (col > 79) col = 79;

		palettes[3][count].r = palettes[3][count].g = palettes[3][count].b =
			col;

	}

	episodes = 11;

	for (count = 0; count < 11; count++) {

		screens[count + 3] = file->loadSurface(134, 110);

		if (file->tell() >= file->getSize()) {

			episodes = ++count;

			for (; count < 11; count++) {

				pixels = new unsigned char[1];
				*pixels = 0;
				screens[count + 3] = createSurface(pixels, 1, 1);

			}

		}

	}

	delete file;

	playMusic("menusng.psm");

	return;

}


Menu::~Menu () {

	int count;

	for (count = 0; count < 15; count++) SDL_FreeSurface(screens[count]);

	return;

}


