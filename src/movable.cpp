
/*
 *
 * movable.cpp
 *
 * 15th January 2005: Created movable.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2009 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Contains the base class for all movable objects.
 *
 */


#include "movable.h"


fixed Movable::getDrawX (int change) {

	return x + ((dx * change) >> 10);

}


fixed Movable::getDrawY (int change) {

	return y + ((dy * change) >> 10);

}


fixed Movable::getX () {

	return x;

}


fixed Movable::getY () {

	return y;

}


