
/**
 *
 * @file jj1bullet.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 4th February 2009: Created events.h from parts of level.h
 * - 11th February 2009: Created bullet.h from parts of events.h
 * - 1st August 2012: Renamed bullet.h to jj1bullet.h
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _BULLET_H
#define _BULLET_H


#include "level/movable.h"

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

class JJ1Bird;
class JJ1Event;
class JJ1LevelPlayer;
class Sprite;

/// JJ1Bullet
class JJ1Bullet : public Movable {

	private:
		JJ1Bullet*      next; ///< The next bullet
		JJ1LevelPlayer* source; ///< Source player. If NULL, was fired by an event
		Sprite*         sprite; ///< Sprite
		signed char*    set; ///< Bullet type properties
		int             direction; ///< 0: Left, 1: Right, 2: L (lower), 3: R (lower)
		unsigned int    time; ///< Time at which the bullet will self-destruct

		JJ1Bullet* remove ();

	public:
		JJ1Bullet  (JJ1Bullet* nextBullet, JJ1LevelPlayer* sourcePlayer, fixed startX, fixed startY, signed char *bullet, int newDirection, unsigned int ticks);
		~JJ1Bullet ();

		JJ1LevelPlayer* getSource ();
		JJ1Bullet*      step      (unsigned int ticks);
		void            draw      (int change);

};

#endif

