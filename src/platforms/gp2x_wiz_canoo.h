
/**
 *
 * @file gp2x_wiz_canoo.h
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


#ifndef GP2X_WIZ_CANOO_H
#define GP2X_WIZ_CANOO_H

#if defined(GP2X) || defined(WIZ)

#define GP2X_BUTTON_VOLUP         (16)
#define GP2X_BUTTON_VOLDOWN       (17)

#define VOLUME_MIN 0
#define VOLUME_MAX 100
#define VOLUME_CHANGE_RATE 2
#define VOLUME_NOCHG 0
#define VOLUME_DOWN 1
#define VOLUME_UP 2

void WIZ_AdjustVolume(int direction);

#endif

// Video
#if defined(GP2X) || defined(WIZ) || defined(CAANOO)
	#define DEFAULT_SCREEN_WIDTH 320
	#define DEFAULT_SCREEN_HEIGHT 240
	#define FULLSCREEN_ONLY
	#define NO_RESIZE
	#define FULLSCREEN_FLAGS (SDL_FULLSCREEN | SDL_SWSURFACE | SDL_HWPALETTE)
#endif

// Buttons
#if defined(GP2X) || defined(WIZ)
	#define DEFAULT_BUTTON_UP     (0)
	#define DEFAULT_BUTTON_DOWN   (4)
	#define DEFAULT_BUTTON_LEFT   (2)
	#define DEFAULT_BUTTON_RIGHT  (6)
	#define DEFAULT_BUTTON_JUMP   (12)    /* A */
	#define DEFAULT_BUTTON_SWIM   (12)    /* A */
	#define DEFAULT_BUTTON_FIRE   (14)    /* X */
	#define DEFAULT_BUTTON_CHANGE (15)    /* Y */
	#define DEFAULT_BUTTON_ESCAPE (10)    /* L */
	#define DEFAULT_BUTTON_ENTER  (11)    /* R */
	#define DEFAULT_BUTTON_PAUSE  (8)     /* Start */
	#define DEFAULT_BUTTON_STATS  (9)     /* Select */
#elif defined(CAANOO)
	#define DEFAULT_BUTTON_JUMP   (0)     /* A? */
	#define DEFAULT_BUTTON_SWIM   (0)     /* A? */
	#define DEFAULT_BUTTON_FIRE   (1)     /* X? */
	#define DEFAULT_BUTTON_CHANGE (3)     /* Y? */
	#define DEFAULT_BUTTON_ESCAPE (6)     /* Home */
	#define DEFAULT_BUTTON_ENTER  (5)     /* R? */
	#define DEFAULT_BUTTON_PAUSE  (9)     /* Help 2 */
	#define DEFAULT_BUTTON_STATS  (8)     /* Help 1 */
#endif

#endif
