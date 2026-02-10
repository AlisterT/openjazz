
/**
 *
 * @file apple.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2023 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _APPLE_H
#define _APPLE_H

#include "platform_interface.h"

#ifdef __APPLE__

class ApplePlatform final : public IPlatform {
	public:
		void AddGamePaths() override;
		void ErrorNoDatafiles() override;
};

#endif

#endif
