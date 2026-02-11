
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

#include "platform_interface.h"

#ifdef __vita__

/* Device uses SDL2 */

// Audio config
#define SOUND_SAMPLES 512

// Keyboard config
#define NO_KEYBOARD_CFG

// Video config
#define DEFAULT_SCREEN_WIDTH 960
#define DEFAULT_SCREEN_HEIGHT 540
#define FULLSCREEN_ONLY
#define NO_RESIZE

// Controller config
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
