
/**
 *
 * @file android.cpp
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

#include "android.h"

#ifdef __ANDROID__

#include <SDL.h>
#include "io/file.h"
#include "util.h"

void ANDROID_AddGamePaths() {
	const char* pref = SDL_GetPrefPath("eu.alister", "openjazz");
	if(pref) {
		gamePaths.add(createString(pref), PATH_TYPE_CONFIG|PATH_TYPE_TEMP);
	}

	const char* storage = SDL_AndroidGetExternalStoragePath();
	if(storage) {
		gamePaths.add(createString(storage), PATH_TYPE_SYSTEM|PATH_TYPE_GAME);
	}
}

#endif
