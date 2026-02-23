
/**
 *
 * @file jj1episodeutils.h
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

#ifndef OJ_EPISODEUTILS_H
#define OJ_EPISODEUTILS_H

#include "OpenJazz.h"

// Functions

int episodeToWorld (int episode);
int worldToEpisode (int world);
const char *episodeTag (int episode);
const char *episodeTitle (int episode);

#endif
