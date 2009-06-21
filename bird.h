
/*
 *
 * bird.h
 *
 * Created on the 1st of March 2009 from parts of events.h
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


#ifndef _BIRD_H
#define _BIRD_H


#include "OpenJazz.h"


// Constants

// Animations
#define BIRD_LEFTANIM  51
#define BIRD_RIGHTANIM 52

// Time interval
#define T_BIRD_FIRE 500


// Classes

class Player;

class Bird {

	private:
		Player *player;
		fixed   x, y, dx, dy;
		bool    fleeing;
		int     fireTime;

	public:
		Bird               (Player *player, unsigned char gX, unsigned char gY);
		~Bird              ();

		void reset         ();
		Player * getPlayer ();
		void     hit       ();
		fixed    getX      ();
		fixed    getY      ();
		bool     playFrame (int ticks);
		void     draw      (int ticks);

};

#endif

