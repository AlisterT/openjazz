
/*
 *
 * jj2event.h
 *
 * 2nd July 2010: Created jj2event.h from parts of jj2level.h
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


#ifndef _JJ2EVENT_H
#define _JJ2EVENT_H


#include "level/movable.h"


// Classes

class Anim;

class JJ2Event : public Movable {

	private:
		JJ2Event* next;

	protected:
		unsigned char type;
		int           properties;
		unsigned int  endTime;  // Point at which the event will terminate
		bool          flipped;

		JJ2Event (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType, int newProperties);

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

class PickupJJ2Event : public JJ2Event {

	private:
		bool floating;

	protected:
		PickupJJ2Event          (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType, int newProperties);
		virtual ~PickupJJ2Event ();

		JJ2Event* step (unsigned int ticks, int msps);

};

class AmmoJJ2Event : public PickupJJ2Event {

	public:
		AmmoJJ2Event  (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType);
		~AmmoJJ2Event ();

		void      draw (unsigned int ticks, int change);

};

class CoinGemJJ2Event : public PickupJJ2Event {

	private:
		void mapPalette (Anim* anim, int start);

	public:
		CoinGemJJ2Event  (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType);
		~CoinGemJJ2Event ();

		void      draw (unsigned int ticks, int change);

};

class FoodJJ2Event : public PickupJJ2Event {

	public:
		FoodJJ2Event  (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType);
		~FoodJJ2Event ();

		void      draw (unsigned int ticks, int change);

};

class OtherJJ2Event : public JJ2Event {

	public:
		OtherJJ2Event  (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char newType, int newProperties);
		~OtherJJ2Event ();

		JJ2Event* step (unsigned int ticks, int msps);
		void      draw (unsigned int ticks, int change);

};

#endif

