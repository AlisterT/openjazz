
/**
 *
 * @file platforms.h
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


#ifndef _PLATFORMS_H
#define _PLATFORMS_H

#include "3ds.h"
#include "apple.h"
#include "dingoo.h"
#include "emscripten.h"
#include "gameshell.h"
#include "gp2x_wiz_canoo.h"
#include "haiku.h"
#include "psp.h"
#include "psvita.h"
#include "riscos.h"
#include "switch.h"
#include "symbian.h"
#include "wii.h"
#include "windows.h"
#include "xdg.h"

inline void PLATFORM_Init() {
#ifdef PSP
	PSP_Init();
#endif

#ifdef __wii__
	WII_Init();
#endif

#ifdef __3DS__
	N3DS_Init();
#endif

#ifdef __SWITCH__
	SWITCH_Init();
#endif

#ifdef __vita__
	PSVITA_Init();
#endif
}

inline void PLATFORM_Exit() {
#ifdef __3DS__
	N3DS_Exit();
#endif

#ifdef __SWITCH__
	SWITCH_Exit();
#endif
}

inline void PLATFORM_AddGamePaths() {
#ifdef __HAIKU__
	HAIKU_AddGamePaths();
#endif

#ifdef __SYMBIAN32__
	SYMBIAN_AddGamePaths();
#endif

#ifdef __3DS__
	N3DS_AddGamePaths();
#endif

#ifdef __SWITCH__
	SWITCH_AddGamePaths();
#endif

#ifdef __wii__
	WII_AddGamePaths();
#endif

#ifdef __vita__
	PSVITA_AddGamePaths();
#endif

#ifdef __riscos__
	RISCOS_AddGamePaths();
#endif

#ifdef __SYMBIAN32__
	SYMBIAN_AddGamePaths();
#endif

// using __unix__ might add too much
#if (__linux__ && !__ANDROID__) || __FreeBSD__ || __OpenBSD__
	#ifndef PORTABLE
	// Only use XDG dirs for installed package
	XDG_AddGamePaths();
	#endif
#endif

#ifdef __APPLE__
	APPLE_AddGamePaths();
#endif
}

inline void PLATFORM_ErrorNoDatafiles() {
#ifdef __HAIKU__
	HAIKU_ErrorNoDatafiles();
#endif

#ifdef PSP
	PSP_ErrorNoDatafiles();
#endif

#ifdef __3DS__
	N3DS_ErrorNoDatafiles();
#endif

#ifdef __SWITCH__
	SWITCH_ErrorNoDatafiles();
#endif

#ifdef __APPLE__
	APPLE_ErrorNoDatafiles();
#endif
}

#endif
