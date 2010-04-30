
/*
 *
 * menuutil.cpp
 *
 * 18th July 2009: Created menuutil.cpp from parts of menu.cpp
 *
 * Part of the OpenJazz project
 *
 *
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

/*
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


int Menu::message (const char* text) {

	// Display a message to the user

	video.setPalette(palettes[1]);

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ENTER) || controls.release(C_ESCAPE))
			return E_NONE;

		SDL_Delay(T_FRAME);

		clearScreen(15);

		// Draw the message
		fontmn2->showString(text, canvasW >> 2, (canvasH >> 1) - 16);

	}

	return E_NONE;

}


int Menu::generic (const char** optionNames, int options, int& chosen) {

	// Let the user select from a menu of the given options

	int count;

	video.setPalette(palettes[1]);

	if (chosen >= options) chosen = 0;

	while (true) {

		if (loop(NORMAL_LOOP) == E_QUIT) return E_QUIT;

		if (controls.release(C_ESCAPE)) return E_UNUSED;

		SDL_Delay(T_FRAME);

		clearScreen(0);

		for (count = 0; count < options; count++) {

			if (count == chosen) fontmn2->mapPalette(240, 8, 114, 16);

			fontmn2->showString(optionNames[count], canvasW >> 2,
				(canvasH >> 1) + (count << 4) - (options << 3));

			if (count == chosen) fontmn2->restorePalette();

		}

		if (controls.release(C_UP)) chosen = (chosen + options - 1) % options;

		if (controls.release(C_DOWN)) chosen = (chosen + 1) % options;

		if (controls.release(C_ENTER)) {

			playSound(S_ORB);

			return E_NONE;

		}

	}

	return E_NONE;

}


int Menu::textInput (const char* request, char*& text) {

	// Let the user to edit a text string

	char *input;
	int count, terminate, character, x;
	unsigned int cursor;

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


		if (controls.release(C_ESCAPE)) {

			delete[] input;

			return E_UNUSED;

		}

		SDL_Delay(T_FRAME);

		clearScreen(15);

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

	return E_UNUSED;

}


