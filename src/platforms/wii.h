
/**
 *
 * @file wii.h
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


#ifndef _WII_H
#define _WII_H

#include "platform_interface.h"

#ifdef __wii__

/* Device uses SDL2 */

// Video config
#define DEFAULT_SCREEN_WIDTH 640
#define DEFAULT_SCREEN_HEIGHT 480
#define FULLSCREEN_ONLY
#define NO_RESIZE

// Controller config
#define DEFAULT_BUTTON_JUMP   (2) /* 1 */
#define DEFAULT_BUTTON_SWIM   (2) /* 1 */
#define DEFAULT_BUTTON_FIRE   (3) /* 2 */
#define DEFAULT_BUTTON_CHANGE (0) /* A */
#define DEFAULT_BUTTON_ENTER  (3) /* 2 */
#define DEFAULT_BUTTON_ESCAPE (6) /* Home */
#define DEFAULT_BUTTON_STATS  (4) /* - */
#define DEFAULT_BUTTON_PAUSE  (5) /* + */

class WiiPlatform final : public IPlatform {
	public:
		WiiPlatform();

		void AddGamePaths() override;
};

#endif

#endif
