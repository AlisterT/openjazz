
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


#ifndef OJ_3DS_H
#define OJ_3DS_H

#include "platform_interface.h"

#ifdef __3DS__

// video config (SDL1.2)

#define DEFAULT_SCREEN_WIDTH 400
#define DEFAULT_SCREEN_HEIGHT 240
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS (SDL_SWSURFACE | SDL_TOPSCR | SDL_CONSOLEBOTTOM)

// audio config

#define SOUND_FREQ 22050
#define SOUND_SAMPLES 1024
#define MUSIC_SETTINGS 1 // mid

// keyboard config

#define NO_KEYBOARD_CFG

// controller config (SDL1.2)

#define DEFAULT_BUTTON_JUMP   (2) /* B */
#define DEFAULT_BUTTON_SWIM   (2) /* B */
#define DEFAULT_BUTTON_FIRE   (4) /* Y */
#define DEFAULT_BUTTON_CHANGE (3) /* X */
#define DEFAULT_BUTTON_ENTER  (1) /* A */
#define DEFAULT_BUTTON_ESCAPE (7) /* Select */
#define DEFAULT_BUTTON_STATS  (5) /* L */
#define DEFAULT_BUTTON_PAUSE  (8) /* Start */

class N3dsPlatform final : public IPlatform {
	public:
		N3dsPlatform();
		~N3dsPlatform();

		void AddGamePaths() override;

		void ErrorNoDatafiles() override;

		bool InputIP(char*& current_ip, char*& new_ip) override;
		bool InputString(const char* hint, char*& current_string, char*& new_string) override;
};

#endif

#endif
