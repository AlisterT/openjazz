
/**
 *
 * @file android.h
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


#ifndef _ANDROID_H
#define _ANDROID_H

#ifdef __ANDROID__

void ANDROID_AddGamePaths();

// video config (SDL2)

#define DEFAULT_SCREEN_WIDTH 640
#define DEFAULT_SCREEN_HEIGHT 480
//#define FULLSCREEN_ONLY

// keyboard config (SDL2)

#define DEFAULT_KEY_ESCAPE SDLK_AC_BACK /* back button */

// controller config (SDL2)

#define DEFAULT_BUTTON_JUMP   (2)
#define DEFAULT_BUTTON_SWIM   (2)
#define DEFAULT_BUTTON_FIRE   (4)
#define DEFAULT_BUTTON_CHANGE (3)
#define DEFAULT_BUTTON_ENTER  (1)
#define DEFAULT_BUTTON_ESCAPE (6)
#define DEFAULT_BUTTON_STATS  (5)
#define DEFAULT_BUTTON_PAUSE  (7)

#endif

#endif
