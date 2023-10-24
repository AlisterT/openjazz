
/**
 *
 * @file 3ds.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "3ds.h"
#include "util.h"

#ifdef _3DS

#include <3ds.h>

void N3DS_Init() {
    // file system
    romfsInit();
}

void N3DS_Exit() {
    romfsExit();
}

void N3DS_AddGamePaths() {
    firstPath = new Path(firstPath, createString("sdmc:/3ds/OpenJazz/"));
    firstPath = new Path(firstPath, createString("romfs:/"));
}

int N3DS_InputIP(char*& current_ip, char*& new_ip) {

	SwkbdState swkbd;
	char ip_buf[16];
	SwkbdButton button = SWKBD_BUTTON_NONE;

	swkbdInit(&swkbd, SWKBD_TYPE_NUMPAD, 2, 15);
	swkbdSetInitialText(&swkbd, current_ip);
	swkbdSetHintText(&swkbd, "Please enter an IP address");
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetFeatures(&swkbd, SWKBD_FIXED_WIDTH);
	swkbdSetNumpadKeys(&swkbd, '.', 0);

	button = swkbdInputText(&swkbd, ip_buf, sizeof(ip_buf));
	if (button == SWKBD_BUTTON_CONFIRM) {
		new_ip = createString(ip_buf);
		return 1;
	}

	return 0;

}

static SwkbdCallbackResult StringFilter(void*, const char** ppMessage, const char* text, size_t textlen) {

	for (size_t i = 0; i < textlen; i++) {

		if ((text[i] != ' ') && (text[i] != '.') &&
			((text[i] < '0') || (text[i] > '9')) &&
			((text[i] < 'a') || (text[i] > 'z'))) {

			*ppMessage = "Invalid character(s) found.\nOnly a-z, 0-9, Dot and Space\nare allowed.";
			return SWKBD_CALLBACK_CONTINUE;

		}

	}

	return SWKBD_CALLBACK_OK;
}

int N3DS_InputString(const char* hint, char*& current_string, char*& new_string) {

	SwkbdState swkbd;
	char string_buf[STRING_LENGTH + 1];
	SwkbdButton button = SWKBD_BUTTON_NONE;

	swkbdInit(&swkbd, SWKBD_TYPE_QWERTY, 2, STRING_LENGTH);
	swkbdSetInitialText(&swkbd, current_string);
	swkbdSetHintText(&swkbd, hint);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetFilterCallback(&swkbd, StringFilter, NULL);

	button = swkbdInputText(&swkbd, string_buf, sizeof(string_buf));
	if (button == SWKBD_BUTTON_CONFIRM) {
		new_string = createString(string_buf);
		return 1;
	}

	return 0;

}

#endif
