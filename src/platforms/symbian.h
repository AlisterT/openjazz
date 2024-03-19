
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


#ifndef OJ_SYMBIAN_H
#define OJ_SYMBIAN_H

#include "platform_interface.h"

#ifdef __SYMBIAN32__

extern float sinf (float);

// audio config

#define SOUND_FREQ 22050
#define MUSIC_SETTINGS 1 // Mid

class SymbianPlatform final : public IPlatform {
	public:
		void AddGamePaths() override;
};

#endif

#endif
