
/**
 *
 * @file plasma.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd June 2010: Created plasma.h
 *
 * @section Licence
 * Copyright (c) 2010 Alireza Nejati
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _PLASMA_H
#define _PLASMA_H

#include <SDL/SDL.h>

/// Main menu background plasma effect
class Plasma {

	private:
		int p0,p1,p2,p3;

	public:
		Plasma ();

		int draw();

};

#endif

