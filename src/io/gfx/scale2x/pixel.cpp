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

#include "pixel.h"

pixel_t pixel_get(int x, int y, const unsigned char* pix, unsigned slice, unsigned pixel, unsigned dx, unsigned dy, int opt_tes)
{
	const unsigned char* p;
	unsigned i;
	pixel_t v;

	if (opt_tes) {
		if (x < 0)
			x += dx;
		if ((unsigned int)x >= dx)
			x -= dx;
		if (y < 0)
			y += dy;
		if ((unsigned int)y >= dy)
			y -= dy;
	} else {
		if (x < 0)
			x = 0;
		if ((unsigned int)x >= dx)
			x = dx - 1;
		if (y < 0)
			y = 0;
		if ((unsigned int)y >= dy)
			y = dy - 1;
	}

	p = pix + (y * slice) + (x * pixel);

	v = 0;
	for(i=0;i<pixel;++i)
		v |= ((pixel_t)p[i]) << (i*8);

	return v;
}

void pixel_put(int x, int y, unsigned char* pix, unsigned slice, unsigned pixel, unsigned dx, unsigned dy, pixel_t v)
{
	unsigned char* p;
	unsigned i;

	p = pix + (y * slice) + (x * pixel);

	for(i=0;i<pixel;++i) {
		p[i] = v >> (i*8);
	}
}
