/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2003 Andrea Mazzoleni
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

#ifndef __PIXEL_H
#define __PIXEL_H

typedef unsigned long long pixel_t;

pixel_t pixel_get(int x, int y, const unsigned char* pix, unsigned slice, unsigned pixel, unsigned dx, unsigned dy, int opt_tes);
void pixel_put(int x, int y, unsigned char* pix, unsigned slice, unsigned pixel, unsigned dx, unsigned dy, pixel_t v);

#endif

