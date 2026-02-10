
/**
 *
 * @file xdg.cpp
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

#include "xdg.h"

#if __unix__

#include <cstring>
#include <sys/stat.h>

#include "util.h"
#include "io/file.h"
#include "io/log.h"

namespace {
	struct {
		const char *env;
		const char *relPath;
		int type;
		bool create;
	} xdgSpec[] = {
		{"XDG_CONFIG_HOME", "/.config", PATH_TYPE_CONFIG, true},
		{"XDG_CACHE_HOME", "/.cache", PATH_TYPE_TEMP, true},
		{"XDG_DATA_HOME", "/.local/share", PATH_TYPE_GAME, false}
	};
}

void XDGPlatform::AddGamePaths() {
	// first get home directory, will be useful later and serves as sanity check
	const char *homeDir = getenv("HOME");
	if (!homeDir) {
		LOG_WARN("No $HOME found, this might break something.");
		return;
	}

	// define our subdirectory
	const char *appDir = "/openjazz";

	// check all needed XDG directories from spec
	for(auto const &entry : xdgSpec) {
		// length of final path
		size_t xdgLen = 0;

		// check for user override
		const char *env = getenv(entry.env);
		if (env) {
			xdgLen = strlen(env);
		} else {
			// use fallback directory
			xdgLen = strlen(homeDir) + strlen(entry.relPath);
		}

		// concatenate path
		char *xdgPath = new char[xdgLen + strlen(appDir) + 1];
		if(env) {
			strcpy(xdgPath, env);
		} else {
			strcpy(xdgPath, homeDir);
			strcat(xdgPath, entry.relPath);
		}
		strcat(xdgPath, appDir);

		// create if needed
		if(entry.create) {
			LOG_TRACE("Trying to create '%s' directory", xdgPath);
			mkdir(xdgPath, 0777);
		}

		gamePaths.add(xdgPath, entry.type);
	}
}

#endif
