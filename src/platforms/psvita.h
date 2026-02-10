
/**
 *
 * @file psvita.h
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


#ifndef _PSPVITA_H
#define _PSPVITA_H

#ifdef __vita__

void PSVITA_Init();

void PSVITA_AddGamePaths();

int PSVITA_InputString(const char* hint, char*& current_string, char*& new_string);

// Video config (SDL2)
#define DEFAULT_SCREEN_WIDTH 960
#define DEFAULT_SCREEN_HEIGHT 540
#define FULLSCREEN_ONLY
#define NO_RESIZE

// Controller config (SDL2)
#define DEFAULT_BUTTON_UP     (8)
#define DEFAULT_BUTTON_DOWN   (6)
#define DEFAULT_BUTTON_LEFT   (7)
#define DEFAULT_BUTTON_RIGHT  (9)
#define DEFAULT_BUTTON_JUMP   (2)
#define DEFAULT_BUTTON_SWIM   (1)
#define DEFAULT_BUTTON_FIRE   (3)
#define DEFAULT_BUTTON_CHANGE (0)
#define DEFAULT_BUTTON_ENTER  (5)
#define DEFAULT_BUTTON_ESCAPE (4)
#define DEFAULT_BUTTON_STATS  (10)
#define DEFAULT_BUTTON_PAUSE  (11)

#endif

#endif
