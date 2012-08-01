
/**
 *
 * @file event.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 4th February 2009: Created events.h from parts of level.h
 * 11th February 2009: Created bullet.h from parts of events.h
 * 1st March 2009: Created bird.h from parts of events.h
 * 19th July 2009: Renamed events.h to event.h
 * 2nd March 2010: Created guardians.h from parts of event.h
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _EVENT_H
#define _EVENT_H


#include "../level.h"
#include "../movable.h"

#include "OpenJazz.h"
#include "../../io/gfx/anim.h"


// Constants

// Animations
#define E_LEFTANIM     0
#define E_RIGHTANIM    1
#define E_LFINISHANIM  2
#define E_RFINISHANIM  3
#define E_LSHOOTANIM   4
#define E_RSHOOTANIM   5
#define E_NOANIM       6

// Delays
#define T_FLASH  100

// Speed factors
#define ES_SLOW ITOF(80)
#define ES_FAST ITOF(240)


// Classes

class JJ1LevelPlayer;

/// JJ1 level event
class JJ1Event : public Movable {

	protected:
		JJ1Event*     next; ///< Next event
		JJ1EventType* set; ///< Type
		unsigned char gridX, gridY; ///< Grid position of the event
		unsigned char animType; ///< Animation type (E_LEFTANIM, etc.)
		unsigned int  flashTime; ///< Time flash will end
		bool          noAnimOffset;

		JJ1Event (unsigned char gX, unsigned char gY);

		JJ1Event* remove  ();
		void      destroy (unsigned int ticks);

		Anim* getAnim   ();
		fixed getHeight ();
		fixed getWidth  ();

		JJ1EventType* prepareStep (unsigned int ticks, int msps);

	public:
		virtual ~JJ1Event ();

		JJ1Event*      getNext        ();
		bool           hit            (JJ1LevelPlayer *source, unsigned int ticks);
		bool           isEnemy        ();
		bool           isFrom         (unsigned char gX, unsigned char gY);
		virtual bool   overlap        (fixed left, fixed top, fixed width, fixed height);

		virtual JJ1Event* step        (unsigned int ticks, int msps) = 0;
		virtual void      draw        (unsigned int ticks, int change) = 0;
		void              drawEnergy  (unsigned int ticks);

};

/// Standard JJ1 level event
class JJ1StandardEvent : public JJ1Event {

	private:
		fixed node; ///< Current event path node
		bool  onlyLAnimOffset;
		bool  onlyRAnimOffset;

		void move (unsigned int ticks, int msps);

	public:
		JJ1StandardEvent (unsigned char gX, unsigned char gY);

		JJ1Event* step (unsigned int ticks, int msps);
		void   draw (unsigned int ticks, int change);

};

/// JJ1 level bridge
class JJ1Bridge : public JJ1Event {

	private:
		fixed leftDipX;
		fixed rightDipX;

	public:
		JJ1Bridge (unsigned char gX, unsigned char gY);

		JJ1Event* step (unsigned int ticks, int msps);
		void   draw (unsigned int ticks, int change);

};

#endif

