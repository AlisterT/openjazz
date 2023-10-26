
/**
 *
 * @file platforms.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _PLATFORMS_H
#define _PLATFORMS_H

#include "psp.h"
#include "3ds.h"
#include "wii.h"
#include "psvita.h"
#include "haiku.h"
#include "wiz.h"
#include "riscos.h"
#include "symbian.h"
#include "xdg.h"

inline void PLATFORM_Init() {
#ifdef PSP
	PSP_Init();
#endif

#ifdef __wii__
	WII_Init();
#endif

#ifdef _3DS
	N3DS_Init();
#endif

#ifdef __vita__
	PSVITA_Init();
#endif
}

inline void PLATFORM_Exit() {
#ifdef _3DS
	N3DS_Exit();
#endif
}

inline void PLATFORM_AddGamePaths() {
#ifdef __HAIKU__
	HAIKU_AddGamePaths();
#endif

#ifdef __SYMBIAN32__
	SYMBIAN_AddGamePaths();
#endif

#ifdef _3DS
	N3DS_AddGamePaths();
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
}

inline void PLATFORM_ErrorNoDatafiles() {
#ifdef __HAIKU__
	HAIKU_ErrorNoDatafiles();
#endif

#ifdef PSP
	PSP_ErrorNoDatafiles();
#endif

	//FIXME: add SDL2 error message box
}

#endif
