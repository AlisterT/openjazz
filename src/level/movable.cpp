
/**
 *
 * @file movable.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 15th January 2010: Created movable.cpp
 *
 * @par Licence:
 * Copyright (c) 2010-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Contains the base class for all movable objects.
 *
 */


#include "level.h"
#include "movable.h"


/**
 * Derive the x-coordinate of the Movable relative to the view coordinates for
 * the current time.
 *
 * @param change Time since last step
 *
 * @return The x-coordinate
 */
fixed Movable::getDrawX (int change) {

	return x + ((dx * change) >> 10) - viewX;

}


/**
 * Derive the y-coordinate of the Movable relative to the view coordinates for
 * the current time.
 *
 * @param change Time since last step
 *
 * @return The y-coordinate
 */
fixed Movable::getDrawY (int change) {

	return y + ((dy * change) >> 10) - viewY;

}


/**
 * Get the basic x-coordinate of the Movable.
 *
 * @return The x-coordinate
 */
fixed Movable::getX () {

	return x;

}


/**
 * Get the basic y-coordinate of the Movable.
 *
 * @return The y-coordinate
 */
fixed Movable::getY () {

	return y;

}


