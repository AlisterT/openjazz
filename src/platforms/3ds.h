
/**
 *
 * @file 3ds.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _3DS_H
#define _3DS_H

#ifdef __3DS__

void N3DS_Init();
void N3DS_Exit();

void N3DS_NetInit();
void N3DS_NetExit();
bool N3DS_NetHasConsole();

void N3DS_AddGamePaths();

void N3DS_ErrorNoDatafiles();

int N3DS_InputIP(char*& current_ip, char*& new_ip);
int N3DS_InputString(const char* hint, char*& current_string, char*& new_string);

#endif

#endif
