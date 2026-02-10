
/**
 *
 * @file symbian.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _SYMBIAN_H
#define _SYMBIAN_H

#include "platform_interface.h"

#ifdef __SYMBIAN32__

extern char KOpenJazzPath[256];
extern float sinf (float);

// Audio config
#define SOUND_FREQ 22050
#define MUSIC_SETTINGS 1 // Mid

class SymbianPlatform final : public IPlatform {
	public:
		void AddGamePaths() override;
};

#endif

#endif
