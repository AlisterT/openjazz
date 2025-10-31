
/**
 *
 * @file switch.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2025 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _SWITCH_H
#define _SWITCH_H

#ifdef __SWITCH__

void SWITCH_Init();
void SWITCH_Exit();

void SWITCH_NetInit();
void SWITCH_NetExit();
bool SWITCH_NetHasConsole();

void SWITCH_AddGamePaths();

void SWITCH_ErrorNoDatafiles();

int SWITCH_InputIP(char*& current_ip, char*& new_ip);
int SWITCH_InputString(const char* hint, char*& current_string, char*& new_string);

#endif

#endif
