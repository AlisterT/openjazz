
/**
 *
 * @file platforms.h
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


#ifndef OJ_PLATFORMS_H
#define OJ_PLATFORMS_H

#include "OpenJazz.h"
#include "3ds.h"
#include "android.h"
#include "emscripten.h"
#include "haiku.h"
#include "opendingux.h"
#include "psp.h"
#include "psvita.h"
#include "riscos.h"
#include "symbian.h"
#include "wii.h"
#include "wiz.h"
#include "xdg.h"

#include "platform_interface.h"

#include <SDL.h>
#if SDL_VERSION_ATLEAST(2, 0, 0)
	#define OJ_SDL2 1
#else
	#define OJ_SDL2 0
#endif

class DefaultPlatform final : public IPlatform {
};

EXTERN IPlatform* platform;

#endif
