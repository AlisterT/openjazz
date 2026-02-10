
/**
 *
 * @file riscos.h
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


#ifndef _RISCOS_H
#define _RISCOS_H

#include "platform_interface.h"

#ifdef __riscos__

class RiscosPlatform final : public IPlatform {
	public:
		void AddGamePaths() override;
};

#endif

#endif
