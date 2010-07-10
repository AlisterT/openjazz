
/*
 *
 * jj2event.cpp
 *
 * 2nd July 2010: Created jj2event.cpp from parts of jj2level.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
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
 * Deals with the creating and freeing of JJ2 events.
 *
 */


#include "jj2event.h"

#include "baselevel.h"


JJ2Event::JJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char* properties) {

	x = TTOF(gridX);
	y = TTOF(gridY);
	dx = 0;
	dy = 0;

	next = newNext;

	type = properties[0];
	data[0] = properties[1];
	data[1] = properties[2];
	data[2] = properties[3];

	hits = 0;
	endTime = 0;

	facing = true;

	return;

}

JJ2Event::~JJ2Event () {

	return;

}


void JJ2Event::destroy (unsigned int ticks) {

	endTime = ticks + 1000;

	return;

}


unsigned char JJ2Event::getType () {

	return type;

}


JJ2Event* JJ2Event::remove () {

	JJ2Event *oldNext;

	oldNext = next;
	next = NULL;
	delete this;

	return oldNext;

}

