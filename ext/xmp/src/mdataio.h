/* Extended Module Player
 * Copyright (C) 1996-2025 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LIBXMP_MDATAIO_H
#define LIBXMP_MDATAIO_H

#include <stddef.h>
#include "common.h"

LIBXMP_BEGIN_DECLS

static inline ptrdiff_t CAN_READ(MFILE *m)
{
	if (m->size >= 0)
		return m->pos >= 0 ? m->size - m->pos : 0;

	return INT_MAX;
}

static inline uint8 mread8(MFILE *m, int *err)
{
	uint8 x = 0xff;
	size_t r = mread(&x, 1, 1, m);
	if (err) {
	    *err = (r == 1) ? 0 : EOF;
	}
	return x;
}

static inline int8 mread8s(MFILE *m, int *err)
{
	int r = mgetc(m);
	if (err) {
	    *err = (r < 0) ? EOF : 0;
	}
	return (int8)r;
}

static inline uint16 mread16l(MFILE *m, int *err)
{
	ptrdiff_t can_read = CAN_READ(m);
	if (can_read >= 2) {
		uint16 n = readmem16l(m->start + m->pos);
		m->pos += 2;
		if(err) *err = 0;
		return n;
	} else {
		m->pos += can_read;
		if(err) *err = EOF;
		return 0xffff;
	}
}

static inline uint16 mread16b(MFILE *m, int *err)
{
	ptrdiff_t can_read = CAN_READ(m);
	if (can_read >= 2) {
		uint16 n = readmem16b(m->start + m->pos);
		m->pos += 2;
		if(err) *err = 0;
		return n;
	} else {
		m->pos += can_read;
		if(err) *err = EOF;
		return 0xffff;
	}
}

static inline uint32 mread24l(MFILE *m, int *err)
{
	ptrdiff_t can_read = CAN_READ(m);
	if (can_read >= 3) {
		uint32 n = readmem24l(m->start + m->pos);
		m->pos += 3;
		if(err) *err = 0;
		return n;
	} else {
		m->pos += can_read;
		if(err) *err = EOF;
		return 0xffffffff;
	}
}

static inline uint32 mread24b(MFILE *m, int *err)
{
	ptrdiff_t can_read = CAN_READ(m);
	if (can_read >= 3) {
		uint32 n = readmem24b(m->start + m->pos);
		m->pos += 3;
		if(err) *err = 0;
		return n;
	} else {
		m->pos += can_read;
		if(err) *err = EOF;
		return 0xffffffff;
	}
}

static inline uint32 mread32l(MFILE *m, int *err)
{
	ptrdiff_t can_read = CAN_READ(m);
	if (can_read >= 4) {
		uint32 n = readmem32l(m->start + m->pos);
		m->pos += 4;
		if(err) *err = 0;
		return n;
	} else {
		m->pos += can_read;
		if(err) *err = EOF;
		return 0xffffffff;
	}
}

static inline uint32 mread32b(MFILE *m, int *err)
{
	ptrdiff_t can_read = CAN_READ(m);
	if (can_read >= 4) {
		uint32 n = readmem32b(m->start + m->pos);
		m->pos += 4;
		if(err) *err = 0;
		return n;
	} else {
		m->pos += can_read;
		if(err) *err = EOF;
		return 0xffffffff;
	}
}

LIBXMP_END_DECLS

#endif
