
/*
 *
 * guardians.h
 *
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


#ifndef _GUARDIANS_H
#define _GUARDIANS_H


#include "event.h"


// Class

class DeckGuardian : public Event {

	private:
		int stage;

	public:
		DeckGuardian (unsigned char gX, unsigned char gY, Event *nextEvent);

		bool overlap (fixed left, fixed top, fixed width, fixed height);
		bool step    (unsigned int ticks, int msps);
		void draw    (unsigned int ticks, int change);

};

#endif

