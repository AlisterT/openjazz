
/**
 *
 * @file SDL_wrapper.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Provides correct path to SDL headers based on version.
 *
 */


#ifndef OJ_SDL_WRAPPER_H
#define OJ_SDL_WRAPPER_H

#if OJ_SDL3
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif

#endif
