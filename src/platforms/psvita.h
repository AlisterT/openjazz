
/**
 *
 * @file psvita.h
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


#ifndef OJ_PSPVITA_H
#define OJ_PSPVITA_H

#include "platform_interface.h"

#ifdef __vita__

// video config (SDL1.2)

#define DEFAULT_SCREEN_WIDTH 960
#define DEFAULT_SCREEN_HEIGHT 540
#define FULLSCREEN_ONLY
#define NO_RESIZE
#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_HWSURFACE)

// audio config

#define SOUND_SAMPLES 512

// keyboard config

#define NO_KEYBOARD_CFG

// controller config (SDL1.2)

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

class VitaPlatform final : public IPlatform {
	public:
		VitaPlatform();

		void AddGamePaths() override;

		bool InputString(const char* hint, char*& current_string, char*& new_string) override;
};

#endif

#endif
