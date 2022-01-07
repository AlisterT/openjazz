
/**
 *
 * @file jj2event.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 29th June 2010: Created jj2level.h from parts of level.h
 * - 2nd July 2010: Created jj2event.h from parts of jj2level.h
 *
 * @par Licence:
 * Copyright (c) 2005-2013 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _JJ2EVENT_H
#define _JJ2EVENT_H


#include "level/movable.h"


// Classes

class Anim;

/// JJ2 level "movable" event
class JJ2Event : public Movable {

	private:
		JJ2Event* next;

	protected:
		unsigned char type;
		int           properties; ///< Event-specific options
		unsigned int  endTime; ///< Point at which the event will terminate
		bool          flipped; ///< Whether or not the sprite image should be flipped

		JJ2Event (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, int newProperties);

		void      destroy     (unsigned int ticks);
		bool      prepareStep (unsigned int ticks, int msps);
		bool      prepareDraw (unsigned int ticks, int change);
		JJ2Event* remove      ();

	public:
		virtual ~JJ2Event ();

		unsigned char     getType ();

		virtual JJ2Event* step    (unsigned int ticks, int msps) = 0;
		virtual void      draw    (unsigned int ticks, int change) = 0;

};

/// JJ2 level pickup event
class PickupJJ2Event : public JJ2Event {

	private:
		bool floating;

	protected:
		unsigned char animSet;

		PickupJJ2Event          (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF, int newProperties);
		virtual ~PickupJJ2Event ();

		JJ2Event* step (unsigned int ticks, int msps);

};

/// JJ2 level ammo
class AmmoJJ2Event : public PickupJJ2Event {

	public:
		AmmoJJ2Event  (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF);
		~AmmoJJ2Event ();

		void      draw (unsigned int ticks, int change);

};

/// JJ2 level gold/silver coin
class CoinGemJJ2Event : public PickupJJ2Event {

	private:
		void mapPalette (Anim* anim, int start);

	public:
		CoinGemJJ2Event  (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF);
		~CoinGemJJ2Event ();

		void      draw (unsigned int ticks, int change);

};

/// JJ2 level food
class FoodJJ2Event : public PickupJJ2Event {

	public:
		FoodJJ2Event  (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF);
		~FoodJJ2Event ();

		void      draw (unsigned int ticks, int change);

};

/// JJ2 level spring
class SpringJJ2Event : public JJ2Event {

	private:
		unsigned char animSet;

	public:
		SpringJJ2Event  (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF, int newProperties);
		~SpringJJ2Event ();

		JJ2Event* step (unsigned int ticks, int msps);
		void      draw (unsigned int ticks, int change);

};


/// Unimplemented JJ2 level event
class OtherJJ2Event : public JJ2Event {

	private:
		unsigned char animSet;

	public:
		OtherJJ2Event  (JJ2Event* newNext, int gridX, int gridY, unsigned char newType, bool TSF, int newProperties);
		~OtherJJ2Event ();

		JJ2Event* step (unsigned int ticks, int msps);
		void      draw (unsigned int ticks, int change);

};

#endif

