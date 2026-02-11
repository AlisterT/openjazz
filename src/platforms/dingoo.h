
/**
 *
 * @file dingoo.h
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


#ifndef _DINGOO_H
#define _DINGOO_H

#ifdef DINGOO

/* Device uses SDL1.2 */

// Audio config
#define MUSIC_SETTINGS 0 // Low

// Video config
#define DEFAULT_SCREEN_WIDTH 320
#define DEFAULT_SCREEN_HEIGHT 240
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS 0

// Controller config
#define NO_CONTROLLER_CFG

// Keyboard config
#define DEFAULT_KEY_JUMP   (SDLK_LCTRL)
#define DEFAULT_KEY_SWIM   (SDLK_LCTRL)
#define DEFAULT_KEY_FIRE   (SDLK_LALT)
#define DEFAULT_KEY_CHANGE (SDLK_LSHIFT)
#define DEFAULT_KEY_ENTER  (SDLK_LCTRL)
#define DEFAULT_KEY_ESCAPE (SDLK_ESCAPE)
#define DEFAULT_KEY_STATS  (SDLK_TAB)
#define DEFAULT_KEY_PAUSE  (SDLK_RETURN)
#define DEFAULT_KEY_YES    (SDLK_LCTRL)
#define DEFAULT_KEY_NO     (SDLK_LALT)

#endif

#endif
