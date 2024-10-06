
/**
 *
 * @file util.h
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created OpenJazz.h
 * - 30th April 2010: Created util.h from parts of OpenJazz.h
 *
 * @par Licence:
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _UTIL_H
#define _UTIL_H

#include "OpenJazz.h"
#include "io/file.h"

// Variable

/// Trigonometric function look-up table
EXTERN fixed sinLut[1024];


// Functions

bool               fileExists           (const char *fileName, int pathType = PATH_TYPE_ANY);
unsigned short int createShort          (const unsigned char* data);
int                createInt            (const unsigned char* data);
char*              createString         (const char *string);
char*              createString         (const char *first, const char *second);
char*              createFileName       (const char *type, int extension);
char*              createFileName       (const char *type, const char *extension);
char*              createFileName       (const char *type, int level, int extension);
char*              createEditableString (const char *string);
void               lowercaseString      (char *string);
void               uppercaseString      (char *string);
void               camelcaseString      (char *string);
fixed              fSin                 (fixed angle);
fixed              fCos                 (fixed angle);
unsigned char*     unpackRLE            (unsigned char* data, unsigned int size, unsigned int outSize);
int                episodeToWorld       (int episode);
int                worldToEpisode       (int world);

#endif
