
/**
 *
 * @file psp.h
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


#ifndef _PSP_H
#define _PSP_H

#ifdef PSP

int PSP_WantsExit();

void PSP_Init();

void PSP_ErrorNoDatafiles();

// video config (SDL1.2)

#define DEFAULT_SCREEN_WIDTH 480
#define DEFAULT_SCREEN_HEIGHT 272
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)

// controller config (SDL1.2)

#define DEFAULT_BUTTON_UP     (8)
#define DEFAULT_BUTTON_DOWN   (6)
#define DEFAULT_BUTTON_LEFT   (7)
#define DEFAULT_BUTTON_RIGHT  (9)
#define DEFAULT_BUTTON_JUMP   (2)
#define DEFAULT_BUTTON_SWIM   (2)
#define DEFAULT_BUTTON_FIRE   (3)
#define DEFAULT_BUTTON_CHANGE (0)
#define DEFAULT_BUTTON_ENTER  (5)
#define DEFAULT_BUTTON_ESCAPE (4)
#define DEFAULT_BUTTON_STATS  (10)
#define DEFAULT_BUTTON_PAUSE  (11)

#endif

#endif
