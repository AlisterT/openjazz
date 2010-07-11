
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


JJ2Event::JJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType, int newProperties) {

	x = TTOF(gridX);
	y = TTOF(gridY);
	dx = 0;
	dy = 0;

	next = newNext;

	type = newType;
	properties = newProperties;

	endTime = 0;

	flipped = false;

	return;

}

JJ2Event::~JJ2Event () {

	return;

}


PickupJJ2Event::PickupJJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType, int newProperties) : JJ2Event(newNext, gridX, gridY, newType, newProperties) {

	floating = true;

	return;

}


PickupJJ2Event::~PickupJJ2Event () {

	return;

}


AmmoJJ2Event::AmmoJJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType) : PickupJJ2Event(newNext, gridX, gridY, newType, 0) {

	return;

}


AmmoJJ2Event::~AmmoJJ2Event () {

	return;

}


CoinGemJJ2Event::CoinGemJJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType) : PickupJJ2Event(newNext, gridX, gridY, newType, 0) {

	return;

}


CoinGemJJ2Event::~CoinGemJJ2Event () {

	return;

}


FoodJJ2Event::FoodJJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType) : PickupJJ2Event(newNext, gridX, gridY, newType, 0) {

	return;

}


FoodJJ2Event::~FoodJJ2Event () {

	return;

}


OtherJJ2Event::OtherJJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType, int newProperties) : JJ2Event(newNext, gridX, gridY, newType, newProperties) {

	return;

}


OtherJJ2Event::~OtherJJ2Event () {

	return;

}


void JJ2Event::destroy (unsigned int ticks) {

	endTime = ticks + 500;

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

