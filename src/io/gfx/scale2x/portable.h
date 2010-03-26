/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001, 2002, 2003 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __PORTABLE_H
#define __PORTABLE_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* ------------------------------------------------------------------------ */
/* getopt */

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if !HAVE_GETOPT
int getopt(int argc, char * const *argv, const char *options);
extern char *optarg;
extern int optind, opterr, optopt;
#endif

#endif

