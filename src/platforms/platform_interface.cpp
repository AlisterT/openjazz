
/**
 *
 * @file platform_interface.cpp
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

#include "platform_interface.h"
#include "platforms.h"

#ifdef OJ_SDL3
	#include <SDL3/SDL.h>
#else
	#include <SDL.h>
#endif

IPlatform::~IPlatform() {}

IPlatform* IPlatform::make() {
#if defined(__HAIKU__)
	return new HaikuPlatform();
#elif defined(PSP)
	return new PspPlatform();
#elif defined(__3DS__)
	return new N3dsPlatform();
#elif defined(__wii__)
	return new WiiPlatform();
#elif defined(__SWITCH__)
	return new SwitchPlatform();
#elif defined(__vita__)
	return new VitaPlatform();
#elif defined(__riscos__)
	return new RiscosPlatform();
#elif defined(__SYMBIAN32__)
	return new SymbianPlatform();
#elif defined(__APPLE__)
	return new ApplePlatform();
#elif defined(__ANDROID__)
	return new AndroidPlatform();
#else
	// using the stuff below
	return new DefaultPlatform();
#endif
}

void IPlatform::AddGamePaths() {
	// using __unix__ might add too much
#if (__linux__ && !__ANDROID__) || __FreeBSD__ || __OpenBSD__
	#ifndef PORTABLE
	// Only use XDG dirs for installed package
	XDGPlatform::AddGamePaths();
	#endif
#endif
}
void IPlatform::ErrorNoDatafiles() {
#if OJ_SDL3 || OJ_SDL2
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
		"OpenJazz",
		"Unable to find game data files. When launching OpenJazz, \n"
		"pass the location of the original game data, eg:\n"
		"  OpenJazz ~/jazz1", nullptr);
#endif
}
