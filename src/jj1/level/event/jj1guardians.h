
/**
 *
 * @file jj1guardians.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 31st January 2006: Created level.h from parts of OpenJazz.h
 * - 4th February 2009: Created events.h from parts of level.h
 * - 19th July 2009: Renamed events.h to event.h
 * - 2nd March 2010: Created guardians.h from parts of event.h
 * - 1st August 2012: Renamed guardians.h to jj1guardians.h
 *
 * @par Licence:
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _GUARDIANS_H
#define _GUARDIANS_H


#include "jj1event.h"


// Class

/// Guardian event base class
class Guardian : public JJ1Event {

	protected:
		int stage;

		Guardian (unsigned char gX, unsigned char gY);

};

/// Episode B guardian
class DeckGuardian : public Guardian {

	public:
		DeckGuardian (unsigned char gX, unsigned char gY);

		bool      overlap (fixed left, fixed top, fixed width, fixed height);
		JJ1Event* step    (unsigned int ticks);
		void      draw    (unsigned int ticks, int change);

};

/// Episode 1 guardian
class MedGuardian : public Guardian {

	private:
		unsigned char direction;
		bool shoot;

	public:
		MedGuardian (unsigned char gX, unsigned char gY);

		//bool   overlap (fixed left, fixed top, fixed width, fixed height);
		JJ1Event* step    (unsigned int ticks);
		void      draw    (unsigned int ticks, int change);

};


#endif

