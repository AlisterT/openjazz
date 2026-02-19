
/**
 *
 * @file setup.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 3rd February 2009: Created menu.h from parts of OpenJazz.h
 * - 21st July 2013: Created setup.h from parts of menu.h
 *
 * @par Licence:
 * Copyright (c) 2005-2013 AJ Thomson
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _SETUP_H
#define _SETUP_H


#include "player/player.h"

#include "OpenJazz.h"

// Available options in config file
struct SetupOptions {
	bool valid;
	int videoWidth;
	int videoHeight;
	bool fullScreen;
	int videoScale;
	scalerType scaleMethod;
};


// Class

/// Configuration
class Setup {

	public:
		char*         characterName;
		unsigned char characterCols[PCOLOURS];
		bool          manyBirds;
		bool          leaveUnneeded;
		bool          slowMotion;
		hudType       hudStyle;

		Setup  ();
		~Setup ();

		SetupOptions load ();
		void save ();

};


// Variable

EXTERN Setup setup;

#endif

