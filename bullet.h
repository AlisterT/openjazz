
/*
 *
 * bullet.h
 *
 * Created on the 11th of February 2009 from parts of events.h
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


#ifndef _BULLET_H
#define _BULLET_H


#include "events.h"
#include "player.h"


// Constants

// Indexes for elements of the bullet set
#define B_SPRITE       0
#define B_XSPEED       4
#define B_YSPEED       8
#define B_GRAVITY     12
#define B_FINISHANIM  16
#define B_FINISHSOUND 17
#define B_BEHAVIOUR   18
#define B_BEHAVIOR    18
#define B_STARTSOUND  19

// Survival time
#define T_BULLET 1000
#define T_TNT    300

// Class

class Bullet {

	private:
		Bullet *next;
		Player *source;    // If NULL, was fired by an event
		int     type;      // -1 is TNT, otherwise this indexes the bullet set
		int     direction; // 0: Left, 1: Right, 2: L (lower), 3: R (lower)
		fixed   x, y, dy;
		int     time;      // The time at which the bullet will self-destruct

	public:
		Bullet              (Player *sourcePlayer, bool lower, int ticks,
			Bullet *next);
		Bullet              (Event *sourceEvent, int ticks, Bullet *next);
		Bullet              (Bird *sourceBird, bool lower, int ticks,
			Bullet *nextBullet);
		~Bullet             ();

		Bullet * getNext    ();
		void     removeNext ();
		Player * getSource  ();
		bool     playFrame  (int ticks);
		void     draw       ();

};

#endif

