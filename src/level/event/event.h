
/*
 *
 * event.h
 *
 * 4th February 2009: Created events.h from parts of level.h
 * 11th February 2009: Created bullet.h from parts of events.h
 * 1st March 2009: Created bird.h from parts of events.h
 * 19th July 2009: Renamed events.h to event.h
 * 2nd March 2010: Created guardians.h from parts of event.h
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


#ifndef _EVENT_H
#define _EVENT_H


#include "../movable.h"

#include "OpenJazz.h"
#include "../../io/gfx/anim.h"


// Constants

// Indexes for elements of the event set
/* Names taken from J1CS/JCS94 and J1E
 * ...Except of course it carries on the fine JCF tradition of changing the
 * spelling of words such as "behavior" */
#define E_DIFFICULTY   0
#define E_REFLECTION   2
#define E_BEHAVIOR     4
#define E_BEHAVIOUR    4
#define E_LEFTANIM     5
#define E_RIGHTANIM    6
#define E_MAGNITUDE    8
#define E_HITSTOKILL   9
#define E_MODIFIER     10
#define E_ADDEDSCORE   11
#define E_BULLET       12
#define E_BULLETSP     13
#define E_MOVEMENTSP   15
#define E_ANIMSP       17
#define E_SOUND        21
#define E_MULTIPURPOSE 22
#define E_YAXIS        23
#define E_BRIDGELENGTH 24
#define E_CHAINLENGTH  25
#define E_CHAINANGLE   26
#define E_LFINISHANIM  28
#define E_RFINISHANIM  29
#define E_LSHOOTANIM   30
#define E_RSHOOTANIM   31

// Delays
#define T_FLASH  100
#define T_FINISH 200
#define T_SHOOT  300

// Speed factors
#define ES_SLOW ITOF(80)
#define ES_FAST ITOF(240)


// Classes

class LevelPlayer;

class Event : public Movable {

	protected:
		Event*        next;
		unsigned char gridX, gridY; // Grid position of the event
		unsigned char animType;     // E_LEFTANIM, etc, or 0
		unsigned char frame;
		unsigned int  flashTime;
		bool 		  onlyLAnimOffset;
		bool		  onlyRAnimOffset;
		bool          noAnimOffset;
		fixed         extraOffset;

		Event ();

		Event*       remove              ();
		void         destroy             (unsigned int ticks);
		fixed        getWidth            ();
		fixed        getHeight           ();
		signed char* prepareStep         (unsigned int ticks, int msps);
		void         onlyLeftAnimOffset  (bool enable);
		void         onlyRightAnimOffset (bool enable);
		void         noOffset            (bool enable);
		void		 setExtraOffset      (fixed offset);

	public:
		Event  (unsigned char gX, unsigned char gY);
		virtual ~Event ();

		Event*         getNext        ();
		bool           hit            (LevelPlayer *source, unsigned int ticks);
		bool           isEnemy        ();
		bool           isFrom         (unsigned char gX, unsigned char gY);
		virtual bool   overlap        (fixed left, fixed top, fixed width, fixed height);
		signed char    getProperty    (unsigned char property);
		Anim*		   getAnim	      (unsigned char property);
		virtual Event* step        	  (unsigned int ticks, int msps);
		virtual void   draw           (unsigned int ticks, int change);
		void           drawEnergy 	  (unsigned int ticks);

};

class Bridge : public Event {

	private:
		fixed leftDipX;
		fixed rightDipX;

	public:
		Bridge (unsigned char gX, unsigned char gY);

		Event* step (unsigned int ticks, int msps);
		void   draw (unsigned int ticks, int change);

};

#endif

