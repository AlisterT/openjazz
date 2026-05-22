
/**
 *
 * @file debug.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Provides utilities for better debugging.
 *
 */


#ifndef OJ_DEBUG_H
#define OJ_DEBUG_H

#if OJ_DEBUG

// Enums

/// Debug menu pages
enum Page {
	DP_SHOWFLAGS = 0,
	DP_CHEATS
};

/// Debug show flags
enum ShowFlags {
	DS_NONE = 0,
	DS_TILEMASK = 1 << 0,
	DS_BGTILES = 1 << 1,
	DS_FGTILES = 1 << 2,
	DS_ANIMTILES = 1 << 3,
	DS_PLAYERAREA = 1 << 4,
	DS_PLAYERBASE = 1 << 5,
	DS_PLAYEREVENTTILE = 1 << 6
};

/// Debug cheat flags
enum CheatFlags {
	DC_NONE = 0,
	DC_INVINCIBLE = 1 << 0,
	DC_AMMO = 1 << 1,
	DC_RUNNING = 1 << 2,
	DC_AIRBOARD = 1 << 3,
	DC_BIRD = 1 << 4,
	DC_LEVELSKIP = 1 << 5,
	DC_BONUS = 1 << 6,
	DC_KILL = 1 << 7
};

namespace debug {

extern int showFlags;
extern int cheatFlags;

}

#endif

#endif
