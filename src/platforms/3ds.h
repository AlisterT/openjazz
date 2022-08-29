
/**
 *
 * @file 3ds.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _3DS_H
#define _3DS_H

#ifdef _3DS

void N3DS_Init();
void N3DS_Exit();

void N3DS_AddGamePaths();

int N3DS_InputIP(char*& current_ip, char*& new_ip);
int N3DS_InputString(const char* hint, char*& current_string, char*& new_string);

#endif

#endif
