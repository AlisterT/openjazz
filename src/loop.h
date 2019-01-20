
/**
 *
 * @file loop.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 30th April 2010: Created loop.h from parts of OpenJazz.h
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _LOOP_H
#define _LOOP_H


#include "io/gfx/paletteeffects.h"


// Constants

// Return values
#define JOYSTICKB    0x100
#define JOYSTICKANEG 0x200
#define JOYSTICKAPOS 0x300
#define JOYSTICKHUP  0x400
#define JOYSTICKHLFT 0x500
#define JOYSTICKHRHT 0x600
#define JOYSTICKHDWN 0x700


// Variable

EXTERN unsigned int globalTicks;


// Enum

/// Ways the loop function should handle input.
enum LoopType {

	NORMAL_LOOP, ///< Normal behaviour
	TYPING_LOOP, ///< Return key presses
	SET_KEY_LOOP, ///< Return key presses without modifying control state
	SET_JOYSTICK_LOOP ///< Return joystick actions without modifying control state

};


// Function in main.cpp

EXTERN int loop (LoopType type, PaletteEffect* paletteEffects = NULL, bool effectsStopped = false);

#endif

