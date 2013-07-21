
/**
 *
 * @file loop.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 30th April 2010: Created loop.h from parts of OpenJazz.h
 *
 * @section Licence
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _LOOP_H
#define _LOOP_H


#include "io/gfx/paletteeffects.h"


// Enum

/// Ways the loop function should handle input.
enum LoopType {

	NORMAL_LOOP, ///< Normal behaviour
	TYPING_LOOP, ///< Return key presses
	SET_KEY_LOOP, ///< Return key presses without modifying control state
	SET_JOYSTICK_LOOP ///< Return joystick actions without modifying control state

};


// Function in main.cpp

EXTERN int loop (LoopType type, PaletteEffect* paletteEffects = NULL);

#endif

