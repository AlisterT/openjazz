
/**
 *
 * @file 3ds.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "3ds.h"

#ifdef __3DS__

#include <3ds.h>
#include "util.h"
#include "io/file.h"

N3dsPlatform::N3dsPlatform() {
	// file system
	romfsInit();
}

N3dsPlatform::~N3dsPlatform() {
	romfsExit();
}

void N3dsPlatform::AddGamePaths() {
	gamePaths.add(createString("sdmc:/3ds/OpenJazz/"), PATH_TYPE_GAME|PATH_TYPE_CONFIG);
	gamePaths.add(createString("romfs:/"), PATH_TYPE_SYSTEM|PATH_TYPE_GAME);
}


void N3dsPlatform::ErrorNoDatafiles() {
	errorConf errCnf;
	const char *error = "Unable to find game data files.\n\n"
		"Game files can be shipped inside RomFS and are searched under\n"
		"sdmc:/3ds/OpenJazz/";

	errorInit(&errCnf, ERROR_TEXT_WORD_WRAP, CFG_LANGUAGE_EN);
	errorText(&errCnf, error);
	errorDisp(&errCnf);
}

bool N3dsPlatform::InputIP(char*& current_ip, char*& new_ip) {

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
		return true;
	}

	return false;

}

namespace {
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
}

bool N3dsPlatform::InputString(const char* hint, char*& current_string, char*& new_string) {

	SwkbdState swkbd;
	char string_buf[STRING_LENGTH + 1];
	SwkbdButton button = SWKBD_BUTTON_NONE;

	swkbdInit(&swkbd, SWKBD_TYPE_QWERTY, 2, STRING_LENGTH);
	swkbdSetInitialText(&swkbd, current_string);
	swkbdSetHintText(&swkbd, hint);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetFilterCallback(&swkbd, StringFilter, nullptr);

	button = swkbdInputText(&swkbd, string_buf, sizeof(string_buf));
	if (button == SWKBD_BUTTON_CONFIRM) {
		new_string = createString(string_buf);
		return true;
	}

	return false;

}

#endif
