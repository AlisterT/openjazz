
/**
 *
 * @file bullet.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 11th February 2009: Created bullet.h from parts of events.h
 *
 * @section Licence
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


#ifndef _BULLET_H
#define _BULLET_H


#include "movable.h"

#include "OpenJazz.h"


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


// Classes

class Bird;
class Event;
class LevelPlayer;
class Sprite;

/// Bullet
class Bullet : public Movable {

	private:
		Bullet*      next; ///< The next bullet
		LevelPlayer* source; ///< Source player. If NULL, was fired by an event
		Sprite*      sprite; ///< Sprite
		int          type; ///< -1 is TNT, otherwise indexes the bullet set
		int          direction; ///< 0: Left, 1: Right, 2: L (lower), 3: R (lower)
		unsigned int time; ///< Time at which the bullet will self-destruct

		Bullet* remove ();

	public:
		Bullet  (LevelPlayer* sourcePlayer, bool lower, unsigned int ticks);
		Bullet  (Bird* sourceBird, bool lower, unsigned int ticks);
		Bullet  (fixed xStart, fixed yStart, unsigned char bullet, bool facing, unsigned int ticks);
		~Bullet ();

		LevelPlayer* getSource  ();
		Bullet*      step       (unsigned int ticks, int msps);
		void         draw       (int change);

};

#endif

