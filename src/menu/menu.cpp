
/**
 *
 * @file menu.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd of August 2005: Created menu.c
 * 3rd of February 2009: Renamed menu.c to menu.cpp
 * 9th March 2009: Created game.cpp from parts of menu.cpp and level.cpp
 * 18th July 2009: Created menugame.cpp from parts of menu.cpp
 * 18th July 2009: Created menuutil.cpp from parts of menu.cpp
 * 18th July 2009: Created menusetup.cpp from parts of menu.cpp
 * 19th July 2009: Created menumain.cpp from parts of menu.cpp
 * 23rd June 2010: Merged menuutil.cpp into menu.cpp
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
 * Provides various generic menus.
 *
 */


#include "menu.h"

#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "loop.h"
#include "util.h"

#include <string.h>


/**
 * Show the "(esc) quits" string.
 */
void Menu::showEscString () {

	fontbig->showString(ESCAPE_STRING, 3, canvasH - 12);

	return;

}


/**
 * Display a message to the user.
 *
 * @param text The message to display
 *
 * @return Error code
 */
int Menu::message (const char* text) {

	video.setPalette(menuPalette);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ENTER) || controls.release(C_ESCAPE) || controls.wasCursorReleased())
			return E_NONE;

		SDL_Delay(T_FRAME);

		video.clearScreen(15);

		// Draw the message
		fontmn2->showString(text, canvasW >> 2, (canvasH >> 1) - 16);

	}

	return E_NONE;

}


/**
 * Let the user select from a menu of the given options.
 *
 * @param optionNames Array of option names
 * @param options The number of options (and size of the names array)
 * @param chosen Which option is selected
 *
 * @return Error code
 */
int Menu::generic (const char** optionNames, int options, int& chosen) {

	int x, y, count;

	if (chosen >= options) chosen = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_RETURN;

		if (controls.release(C_UP)) chosen = (chosen + options - 1) % options;

		if (controls.release(C_DOWN)) chosen = (chosen + 1) % options;

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			return E_NONE;

		}

		if (controls.getCursor(x, y)) {

			if ((x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased()) return E_RETURN;

			x -= canvasW >> 2;
			y -= (canvasH >> 1) - (options << 3);

			if ((x >= 0) && (x < 256) && (y >= 0) && (y < (options << 4))) {

				chosen = y >> 4;

				if (controls.wasCursorReleased()) {

					playSound(S_ORB);

					return E_NONE;

				}

			}

		}

		SDL_Delay(T_FRAME);

		video.clearScreen(0);

		for (count = 0; count < options; count++) {

			if (count == chosen) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(optionNames[count], canvasW >> 2,
				(canvasH >> 1) + (count << 4) - (options << 3));

			if (count == chosen) fontmn2->restorePalette();

		}

		showEscString();

	}

	return E_NONE;

}


/**
 * Let the user edit a text string
 *
 * @param request Description of the text string
 * @param text The text string to be edited
 *
 * @return Error code
 */
int Menu::textInput (const char* request, char*& text) {

	char *input;
	int count, terminate, character, x, y;
	unsigned int cursor;

	video.setPalette(menuPalette);

	// Create input string
	input = createEditableString(text);

	cursor = strlen(input);

	while (true) {

		character = loop(TYPING_LOOP);

		if (character == E_QUIT) {

			delete[] input;

			return E_QUIT;

		}

		// Ensure there is space for another character
		if (cursor < STRING_LENGTH) {

			terminate = (input[cursor] == 0);

			// If the character is valid, add it to the input string

			if ((character == ' ') || (character == '.') ||
				((character >= '0') && (character <= '9')) ||
				((character >= 'a') && (character <= 'z'))) {

				input[cursor] = character;
				cursor++;
				if (terminate) input[cursor] = 0;

			} else if ((character >= 'A') && (character <= 'Z')) {

				input[cursor] = character | 32;
				cursor++;
				if (terminate) input[cursor] = 0;

			}

		}

		if ((character == SDLK_DELETE) && (cursor < strlen(input))) {

			for (count = cursor; count < STRING_LENGTH; count++)
				input[count] = input[count + 1];

		}

		if ((character == SDLK_BACKSPACE) && (cursor > 0)) {

			for (count = cursor - 1; count < STRING_LENGTH; count++)
				input[count] = input[count + 1];

			cursor--;

		}


		if (controls.release(C_ESCAPE) ||
			(controls.getCursor(x, y) && (x < 100) && (y >= canvasH - 12) && controls.wasCursorReleased())) {

			delete[] input;

			return E_RETURN;

		}


		SDL_Delay(T_FRAME);

		video.clearScreen(15);

		// Draw the prompt
		fontmn2->showString(request, canvasW >> 2, (canvasH >> 1) - 16);

		// Draw the section of the text before the cursor
		fontmn2->mapPalette(240, 8, 114, 16);
		terminate = input[cursor];
		input[cursor] = 0;
		x = fontmn2->showString(input, (canvasW >> 2) + 8, canvasH >> 1);

		// Draw the cursor
		drawRect(x, (canvasH >> 1) + 10, 8, 2, 79);

		// Draw the section of text after the cursor
		input[cursor] = terminate;
		fontmn2->showString(input + cursor, x, canvasH >> 1);
		fontmn2->restorePalette();

		showEscString();


		if (controls.release(C_LEFT) && (cursor > 0)) cursor--;

		if (controls.release(C_RIGHT) && (cursor < strlen(input))) cursor++;

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			// Replace the original string with the input string
			delete[] text;
			text = input;

			return E_NONE;

		}

	}

	delete[] input;

	return E_RETURN;

}

