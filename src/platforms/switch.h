
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

#include "platform_interface.h"

#ifdef __SWITCH__

/* Device uses SDL2 */

// Video config
#define DEFAULT_SCREEN_WIDTH 1280
#define DEFAULT_SCREEN_HEIGHT 720
#define FULLSCREEN_ONLY

// Controller config
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

class SwitchPlatform final : public IPlatform {
	public:
		SwitchPlatform();
		~SwitchPlatform();

		void NetInit() override;
		void NetExit() override;
		bool NetHasConsole() override;

		void AddGamePaths() override;
		void ErrorNoDatafiles() override;

		bool InputIP(char*& current_ip, char*& new_ip) override;
		bool InputString(const char* hint, char*& current_string, char*& new_string) override;
};

#endif

#endif
