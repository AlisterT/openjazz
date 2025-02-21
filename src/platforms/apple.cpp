
/**
 *
 * @file apple.cpp
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

#include "apple.h"

#ifdef __APPLE__

#include "TargetConditionals.h"

#include <SDL.h>
#include "util.h"
#include "io/file.h"

void APPLE_AddGamePaths() {
#ifdef TARGET_OS_MAC
	gamePaths.add(createString(SDL_GetPrefPath("", "OpenJazz")), PATH_TYPE_SYSTEM|PATH_TYPE_GAME|PATH_TYPE_CONFIG);
#endif
}

void APPLE_ErrorNoDatafiles() {
#ifdef TARGET_OS_MAC
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenJazz", createString("Unable to find game data files.\n\nPut the data into the folder: ", SDL_GetPrefPath("", "OpenJazz")), 0);
#endif
}

#endif
