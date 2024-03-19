
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


#ifndef OJ_PSP_H
#define OJ_PSP_H

#include "platform_interface.h"

#ifdef PSP

// video config (SDL1.2)

#define DEFAULT_SCREEN_WIDTH 480
#define DEFAULT_SCREEN_HEIGHT 272
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)

// audio config

#define SOUND_FREQ 22050
#define SOUND_SAMPLES 1024
#define MUSIC_SETTINGS 0 // Low

// keyboard config

#define NO_KEYBOARD_CFG

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

class PspPlatform final : public IPlatform {
	public:
		PspPlatform();

		bool WantsExit() override;

		void ErrorNoDatafiles() override;
};

#endif

#endif
