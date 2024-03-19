
/**
 *
 * @file riscos.cpp
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

#include "riscos.h"

#ifdef __riscos__

#include "util.h"
#include "io/file.h"

void RiscosPlatform::AddGamePaths() {
	gamePaths.add(createString("/<Choices$Write>/OpenJazz/"), PATH_TYPE_CONFIG);
}

#endif
