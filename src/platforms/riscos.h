
/**
 *
 * @file riscos.h
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


#ifndef OJ_RISCOS_H
#define OJ_RISCOS_H

#include "platform_interface.h"

#ifdef __riscos__

class RiscosPlatform final : public IPlatform {
	public:
		void AddGamePaths() override;
};

#endif

#endif
