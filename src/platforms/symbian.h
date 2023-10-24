
/**
 *
 * @file symbian.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _SYMBIAN_H
#define _SYMBIAN_H

#ifdef __SYMBIAN32__

extern char KOpenJazzPath[256];
extern float sinf (float);

void SYMBIAN_AddGamePaths();

#endif

#endif
