
/*
 *
 * events.h
 *
 * Created on the 4th of February 2009 from parts of level.h
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


#ifndef _EVENTS_H
#define _EVENTS_H


#include "player.h"


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

// Speed factors
#define ES_SLOW 80
#define ES_FAST 240


// Class

class Event {

	private:
		Event         *next;
		unsigned char  gridX, gridY; // Grid position of the event
		fixed          x, y;         // Actual position of the event
		unsigned char  animType;     // E_LEFTANIM, etc, or 0
		unsigned char  frame;
		int            flashTime;

	public:
		Event                   (unsigned char gX, unsigned char gY,
			Event *nextEvent);
		~Event                  ();

		Event *     getNext     ();
		void        removeNext  ();
		bool        hit         (Player *source, int ticks);
		bool        isFrom      (unsigned char gX, unsigned char gY);
		fixed       getX        ();
		fixed       getY        ();
		fixed       getWidth    ();
		fixed       getHeight   ();
		bool        overlap     (fixed left, fixed top, fixed width,
			fixed height);
		signed char getProperty (unsigned char property);
		bool        getFacing   ();
		bool        playFrame   (int ticks);
		void        draw        (int ticks);

};

#endif

