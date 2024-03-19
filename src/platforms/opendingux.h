
/**
 *
 * @file opendingux.h
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


#ifndef OJ_OPENDINGUX_H
#define OJ_OPENDINGUX_H

// video configuration

#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(GAMESHELL) || defined(DINGOO)
	#define DEFAULT_SCREEN_WIDTH 320
	#define DEFAULT_SCREEN_HEIGHT 240

	#define FULLSCREEN_ONLY
	#define NO_RESIZE

	#ifdef DINGOO
		#define FULLSCREEN_FLAGS 0
	#else
		#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
	#endif
#endif

// audio config

#ifdef GP2X
	#define SOUND_SAMPLES 512
#endif
#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(DINGOO)
	#define MUSIC_SETTINGS 0 // Low
#endif

// keyboard configuration

#if defined(CAANOO) || defined(WIZ) || defined(GP2X)
	#define NO_KEYBOARD_CFG
#endif

#ifdef DINGOO
	#define DEFAULT_KEY_JUMP      (SDLK_LCTRL)
	#define DEFAULT_KEY_SWIM      (SDLK_LCTRL)
	#define DEFAULT_KEY_FIRE      (SDLK_LALT)
	#define DEFAULT_KEY_CHANGE    (SDLK_LSHIFT)
	#define DEFAULT_KEY_ENTER     (SDLK_LCTRL)
	#define DEFAULT_KEY_ESCAPE    (SDLK_ESCAPE)
	#define DEFAULT_KEY_STATS     (SDLK_TAB)
	#define DEFAULT_KEY_PAUSE     (SDLK_RETURN)
	#define DEFAULT_KEY_YES       (SDLK_LCTRL)
	#define DEFAULT_KEY_NO        (SDLK_LALT)
#elif defined(GAMESHELL)
	#define DEFAULT_KEY_JUMP      (SDLK_j)      /* A button */
	#define DEFAULT_KEY_SWIM      (SDLK_j)      /* A button */
	#define DEFAULT_KEY_FIRE      (SDLK_k)      /* B button */
	#define DEFAULT_KEY_CHANGE    (SDLK_o)      /* Y button */
	#define DEFAULT_KEY_ENTER     (SDLK_RETURN) /* START button */
	#define DEFAULT_KEY_ESCAPE    (SDLK_ESCAPE) /* MENU button */
	#define DEFAULT_KEY_STATS     (SDLK_SPACE)  /* SELECT button */
	#define DEFAULT_KEY_PAUSE     (SDLK_p)
	#define DEFAULT_KEY_YES       (SDLK_y)
	#define DEFAULT_KEY_NO        (SDLK_n)
#endif

// controller configuration

#ifdef DINGOO
	#define NO_CONTROLLER_CFG
#endif

#if defined(GP2X) || defined(WIZ)
	#define DEFAULT_BUTTON_UP     (0)
	#define DEFAULT_BUTTON_DOWN   (4)
	#define DEFAULT_BUTTON_LEFT   (2)
	#define DEFAULT_BUTTON_RIGHT  (6)
	#define DEFAULT_BUTTON_JUMP   (12) /* A */
	#define DEFAULT_BUTTON_SWIM   (12) /* A */
	#define DEFAULT_BUTTON_FIRE   (14) /* X */
	#define DEFAULT_BUTTON_CHANGE (15) /* Y */
	#define DEFAULT_BUTTON_ESCAPE (10) /* L */
	#define DEFAULT_BUTTON_ENTER  (11) /* R */
	#define DEFAULT_BUTTON_PAUSE  (8)  /* Start */
	#define DEFAULT_BUTTON_STATS  (9)  /* Select */
#elif defined(CAANOO)
	#define DEFAULT_BUTTON_JUMP   (0)  /* A? */
	#define DEFAULT_BUTTON_SWIM   (0)  /* A? */
	#define DEFAULT_BUTTON_FIRE   (1)  /* X? */
	#define DEFAULT_BUTTON_CHANGE (3)  /* Y? */
	#define DEFAULT_BUTTON_ESCAPE (6)  /* Home */
	#define DEFAULT_BUTTON_ENTER  (5)  /* R? */
	#define DEFAULT_BUTTON_PAUSE  (9)  /* Help 2 */
	#define DEFAULT_BUTTON_STATS  (8)  /* Help 1 */
#endif

#endif
