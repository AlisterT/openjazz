
/**
 *
 * @file psvita.h
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


#ifndef _PSPVITA_H
#define _PSPVITA_H

#ifdef __vita__

void PSVITA_Init();

void PSVITA_AddGamePaths();

int PSVITA_InputString(const char* hint, char*& current_string, char*& new_string);

#endif

#endif
