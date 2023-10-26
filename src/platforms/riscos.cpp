
/**
 *
 * @file riscos.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "riscos.h"

#ifdef __riscos__

#include "util.h"
#include "io/file.h"

void RISCOS_AddGamePaths() {
	gamePaths.add(createString("/<Choices$Write>/OpenJazz/"), PATH_TYPE_CONFIG);
}

#endif
