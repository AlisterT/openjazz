
/**
 *
 * @file switch.h
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


#ifndef _SWITCH_H
#define _SWITCH_H

#ifdef __SWITCH__

void SWITCH_Init();
void SWITCH_Exit();

void SWITCH_NetInit();
void SWITCH_NetExit();
bool SWITCH_NetHasConsole();

void SWITCH_AddGamePaths();

void SWITCH_ErrorNoDatafiles();

int SWITCH_InputIP(char*& current_ip, char*& new_ip);
int SWITCH_InputString(const char* hint, char*& current_string, char*& new_string);

// Video config (SDL2)
#define DEFAULT_SCREEN_WIDTH 1280
#define DEFAULT_SCREEN_HEIGHT 720
#define FULLSCREEN_ONLY

// Controller config (SDL2)
#define DEFAULT_BUTTON_UP     (13)
#define DEFAULT_BUTTON_DOWN   (15)
#define DEFAULT_BUTTON_LEFT   (12)
#define DEFAULT_BUTTON_RIGHT  (14)
#define DEFAULT_BUTTON_JUMP   (1)    /* B */
#define DEFAULT_BUTTON_SWIM   (1)    /* B */
#define DEFAULT_BUTTON_FIRE   (3)    /* Y */
#define DEFAULT_BUTTON_CHANGE (2)    /* X */
#define DEFAULT_BUTTON_ENTER  (0)    /* A */
#define DEFAULT_BUTTON_ESCAPE (10)   /* + */
#define DEFAULT_BUTTON_STATS  (6)    /* L */
#define DEFAULT_BUTTON_PAUSE  (11)   /* - */

#endif

#endif
