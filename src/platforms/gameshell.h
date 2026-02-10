
/**
 *
 * @file gameshell.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _GAMESHELL_H
#define _GAMESHELL_H

#ifdef GAMESHELL

// Video config (SDL1.2)
#define DEFAULT_SCREEN_WIDTH 320
#define DEFAULT_SCREEN_HEIGHT 240
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)

// Keyboard config (SDL1.2)
#define DEFAULT_KEY_JUMP   (SDLK_j)      /* A button */
#define DEFAULT_KEY_SWIM   (SDLK_j)      /* A button */
#define DEFAULT_KEY_FIRE   (SDLK_k)      /* B button */
#define DEFAULT_KEY_CHANGE (SDLK_o)      /* Y button */
#define DEFAULT_KEY_ENTER  (SDLK_RETURN) /* START button */
#define DEFAULT_KEY_ESCAPE (SDLK_ESCAPE) /* MENU button */
#define DEFAULT_KEY_STATS  (SDLK_SPACE)  /* SELECT button */
#define DEFAULT_KEY_PAUSE  (SDLK_p)
#define DEFAULT_KEY_YES    (SDLK_y)
#define DEFAULT_KEY_NO     (SDLK_n)

#endif

#endif
