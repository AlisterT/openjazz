
/**
 *
 * @file jj1episodeutils.cpp
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

#include "jj1episodeutils.h"
#include "io/log.h"

int episodeToWorld (int episode) {
	if(episode < 0 || episode > 9) {
		LOG_WARN("Invalid Episode");

		return 0;
	}

	// 1-6
	if (episode < 6)
		return episode * 3;

	// A-C
	if (episode < 9)
		return (episode + 4) * 3;

	// X
	return 50;
}


int worldToEpisode (int world) {
	if(world < 0 || world > 50) {
		LOG_WARN("Invalid World");

		return 0;
	}

	// X
	if (world == 50)
		return 9;

	int episode = world / 3;

	// A-C
	if (episode > 8)
		return episode - 4;

	// 1-6
	return episode;
}

const char *episodeTag (int episode) {
	constexpr const char* episodeTag[10] = { "1", "2", "3", "4", "5", "6",
		"a", "b", "c", "x" };

	if(episode < 0 || episode > 9) {
		LOG_WARN("Invalid Episode");

		return " ";
	}

	return episodeTag[episode];
}

const char *episodeTitle (int episode) {
	constexpr const char *episodeTitle[10] = { "turtle terror", "ballistic bunny",
		"rabbits revenge", "gene machine", "the chase is on", "the final clash",
		"outta dis world", "turtle soup", "wild wabbit", "holiday hare" };

	if(episode < 0 || episode > 9) {
		LOG_WARN("Invalid Episode");

		return " ";
	}

	return episodeTitle[episode];
}
