
/**
 *
 * @file jj2event.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created level.c
 * - 22nd July 2008: Created levelload.c from parts of level.c
 * - 3rd February 2009: Renamed level.c to level.cpp
 * - 19th July 2009: Added parts of levelload.cpp to level.cpp
 * - 29th June 2010: Created jj2level.cpp from parts of level.cpp
 * - 2nd July 2010: Created jj2event.cpp from parts of jj2level.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with the creating and freeing of JJ2 events.
 *
 */


#include "jj2event.h"

#include "level/level.h"


/**
 * Create event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param newProperties Event properties
 */
JJ2Event::JJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, int newProperties) {

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


/**
 * Delete all events
 */
JJ2Event::~JJ2Event () {

	if (next) delete next;

	return;

}


/**
 * Create pickup event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param TSF Whether or not the level uses TSF animations
 * @param newProperties Event properties
 */
PickupJJ2Event::PickupJJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF, int newProperties) : JJ2Event(newNext, gridX, gridY, newType, newProperties) {

	floating = true;
	animSet = TSF? 71: 67;

	return;

}


/**
 * Delete pickup event
 */
PickupJJ2Event::~PickupJJ2Event () {

	return;

}


/**
 * Create ammo pickup event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param TSF Whether or not the level uses TSF animations
 */
AmmoJJ2Event::AmmoJJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF) : PickupJJ2Event(newNext, gridX, gridY, newType, TSF, 0) {

	return;

}


/**
 * Delete ammo pickup event
 */
AmmoJJ2Event::~AmmoJJ2Event () {

	return;

}


/**
 * Create coin/gem pickup event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param TSF Whether or not the level uses TSF animations
 */
CoinGemJJ2Event::CoinGemJJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF) : PickupJJ2Event(newNext, gridX, gridY, newType, TSF, 0) {

	return;

}


/**
 * Delete coin/gem pickup event
 */
CoinGemJJ2Event::~CoinGemJJ2Event () {

	return;

}


/**
 * Create food pickup event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param TSF Whether or not the level uses TSF animations
 */
FoodJJ2Event::FoodJJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF) : PickupJJ2Event(newNext, gridX, gridY, newType, TSF, 0) {

	return;

}


/**
 * Delete food pickup event
 */
FoodJJ2Event::~FoodJJ2Event () {

	return;

}


/**
 * Create spring event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param TSF Whether or not the level uses TSF animations
 * @param newProperties Event properties
 */
SpringJJ2Event::SpringJJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF, int newProperties) : JJ2Event(newNext, gridX, gridY, newType, newProperties) {

	animSet = TSF? 96: 92;

	return;

}


/**
 * Delete spring event
 */
SpringJJ2Event::~SpringJJ2Event () {

	return;

}


/**
 * Create placeholder event
 *
 * @param newNext Next event
 * @param gridX X-coordinate
 * @param gridY Y-coordinate
 * @param newType Event type
 * @param TSF Whether or not the level uses TSF animations
 * @param newProperties Event properties
 */
OtherJJ2Event::OtherJJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF, int newProperties) : JJ2Event(newNext, gridX, gridY, newType, newProperties) {

	animSet = TSF? 71: 67;

	return;

}


/**
 * Delete placeholder event
 */
OtherJJ2Event::~OtherJJ2Event () {

	return;

}


/**
 * Initiate the destruction of the event
 *
 * @param ticks Time
 */
void JJ2Event::destroy (unsigned int ticks) {

	endTime = ticks + 500;

	return;

}


/**
 * Get the event's type
 *
 * @return Event type
 */
unsigned char JJ2Event::getType () {

	return type;

}


/**
 * Delete this event
 *
 * @return The next event
 */
JJ2Event* JJ2Event::remove () {

	JJ2Event *oldNext;

	oldNext = next;
	next = NULL;
	delete this;

	return oldNext;

}

