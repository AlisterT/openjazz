
/**
 *
 * @file haiku.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _HAIKU_H
#define _HAIKU_H

#include "platform_interface.h"

#ifdef __HAIKU__

class HaikuPlatform final : public IPlatform {
	public:
		void AddGamePaths() override;
		void ErrorNoDatafiles() override;
};

#endif

#endif
