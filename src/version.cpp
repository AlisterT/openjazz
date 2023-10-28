
/**
 *
 * @file version.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2023 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Contains version information.
 *
 */

#include "version.h"

// Fallback version data, should be defined by the build system
#ifndef OJ_DATE
	#define OJ_DATE __DATE__
#endif
#ifndef OJ_VERSION
	#define OJ_VERSION "git"
#endif

const char *oj_date = OJ_DATE;

#ifdef OJ_VERSION_GIT
	const char *oj_version = OJ_VERSION " (" OJ_VERSION_GIT ")";
#else
	const char *oj_version = OJ_VERSION;
#endif
