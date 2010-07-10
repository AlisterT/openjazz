
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

class JJ2Event : public Movable {

	private:
		JJ2Event*     next;
		unsigned char type;
		unsigned char data[3];
		unsigned char hits;  // Number of times the event has been shot
		int           endTime;  // Point at which the event will terminate
		unsigned char anim;
		unsigned char frame;
		unsigned int  flashTime;
		bool          facing;

		JJ2Event* remove  ();
		void      destroy (unsigned int ticks);

	public:
		JJ2Event  (JJ2Event* newNext, unsigned char gridX, unsigned char gridY, unsigned char* properties);
		~JJ2Event ();

		unsigned char getType ();

		JJ2Event*     step    (int ticks, int msps);
		void          draw    (unsigned int ticks, int change);

};

#endif

