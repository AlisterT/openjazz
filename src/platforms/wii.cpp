
/**
 *
 * @file wii.cpp
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

#include "wii.h"

#ifdef __wii__

#include <gccore.h>
#include <fat.h>

#include "util.h"
#include "io/file.h"

void WII_Init() {
	// file system is enabled in SDL2Main

	// USBGecko Debugging
	constexpr int SLOT = CARD_SLOTB;
	if (usb_isgeckoalive(SLOT)) {
		usb_flush(SLOT);
		CON_EnableGecko(SLOT, false);
	}
#ifndef NDEBUG
	// Dolphin UART Debugging
	else if(!__system_argv->argc) {
		SYS_STDIO_Report(true);
	}
#endif
}

void WII_AddGamePaths() {
	gamePaths.add(createString("sd:/apps/OpenJazz/"), PATH_TYPE_GAME|PATH_TYPE_CONFIG);
}

#endif
