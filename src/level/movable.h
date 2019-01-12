
/**
 *
 * @file movable.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 15th January 2010: Created movable.h
 *
 * @par Licence:
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _MOVABLE_H
#define _MOVABLE_H


#include "OpenJazz.h"


// Class

/// Base class for all movable objects (players, events, bullets, birds)
class Movable {

	protected:
		fixed x, y, dx, dy;

		fixed getDrawX (int change);
		fixed getDrawY (int change);

	public:
		fixed getX ();
		fixed getY ();

};


#endif

