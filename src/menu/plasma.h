
/**
 *
 * @file plasma.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd June 2010: Created plasma.h
 *
 * @par Licence:
 * Copyright (c) 2010 Alireza Nejati
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#ifndef _PLASMA_H
#define _PLASMA_H

/// Main menu background plasma effect
class Plasma {

	private:
		int p0,p1,p2,p3;

	public:
		Plasma ();

		int draw();

};

#endif

