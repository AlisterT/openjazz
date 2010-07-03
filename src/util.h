
/*
 *
 * util.h
 *
 * 30th April 2010: Created util.h from parts of OpenJazz.h
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _UTIL_H
#define _UTIL_H


#include "OpenJazz.h"


// Variable

// Trigonometric function look-up table
EXTERN fixed sinLut[1024];


// Functions

EXTERN bool               fileExists           (const char *fileName);
EXTERN unsigned short int createShort          (unsigned char* data);
EXTERN int                createInt            (unsigned char* data);
EXTERN char*              createString         (const char *string);
EXTERN char*              createString         (const char *first, const char *second);
EXTERN char*              createFileName       (const char *type, int extension);
EXTERN char*              createFileName       (const char *type, const char *extension);
EXTERN char*              createFileName       (const char *type, int level, int extension);
EXTERN char*              createEditableString (const char *string);
EXTERN void               log                  (const char *message);
EXTERN void               log                  (const char *message, const char *detail);
EXTERN void               log                  (const char *message, int number);
EXTERN void               logError             (const char *message, const char *detail);
EXTERN fixed              fSin                 (fixed angle);
EXTERN fixed              fCos                 (fixed angle);

#endif

