
/**
 *
 * @file 3ds.h
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


#ifndef _3DS_H
#define _3DS_H

#ifdef _3DS

void N3DS_Init();
void N3DS_Exit();

void N3DS_AddGamePaths();

void N3DS_ErrorNoDatafiles();

int N3DS_InputIP(char*& current_ip, char*& new_ip);
int N3DS_InputString(const char* hint, char*& current_string, char*& new_string);

// video config (SDL1.2)

#define DEFAULT_SCREEN_WIDTH 400
#define DEFAULT_SCREEN_HEIGHT 240
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS (SDL_SWSURFACE | SDL_TOPSCR | SDL_CONSOLEBOTTOM)

// controller config (SDL1.2)

#define DEFAULT_BUTTON_JUMP   (2) /* B */
#define DEFAULT_BUTTON_SWIM   (2) /* B */
#define DEFAULT_BUTTON_FIRE   (4) /* Y */
#define DEFAULT_BUTTON_CHANGE (3) /* X */
#define DEFAULT_BUTTON_ENTER  (1) /* A */
#define DEFAULT_BUTTON_ESCAPE (7) /* Select */
#define DEFAULT_BUTTON_STATS  (5) /* L */
#define DEFAULT_BUTTON_PAUSE  (8) /* Start */

#endif

#endif
