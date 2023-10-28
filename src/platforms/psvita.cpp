
/**
 *
 * @file psvita.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2023 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "psvita.h"
#include "util.h"

#ifdef __vita__

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <kbdvita.h>

#include "util.h"
#include "io/file.h"

void PSVITA_Init() {

	// controls

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

}

void PSVITA_AddGamePaths() {
	gamePaths.add(createString("ux0:data/jazz/"), PATH_TYPE_CONFIG);
}

int PSVITA_InputString(const char* hint, char*& current_string, char*& new_string) {

	char* text = kbdvita_get(hint, current_string, STRING_LENGTH);
	if (text != NULL) {
		new_string = createString(text);
		return 1;
	}

	return 0;

}

#endif
