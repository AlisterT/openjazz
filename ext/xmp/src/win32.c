/* Extended Module Player
 * Copyright (C) 1996-2021 Claudio Matsuoka and Hipolito Carraro Jr
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

/* _[v]snprintf() from msvcrt.dll might not nul terminate */
/* OpenWatcom-provided versions seem to behave the same.. */

#include "common.h"

#if defined(USE_LIBXMP_SNPRINTF)

#undef snprintf
#undef vsnprintf

int libxmp_vsnprintf(char *str, size_t sz, const char *fmt, va_list ap)
{
	int rc = _vsnprintf(str, sz, fmt, ap);
	if (sz != 0) {
		if (rc < 0) rc = (int)sz;
		if ((size_t)rc >= sz) str[sz - 1] = '\0';
	}
	return rc;
}

int libxmp_snprintf (char *str, size_t sz, const char *fmt, ...)
{
	va_list ap;
	int rc;

	va_start (ap, fmt);
	rc = _vsnprintf(str, sz, fmt, ap);
	va_end (ap);

	return rc;
}

#endif

/* Win32 debug message helper by Mirko Buffoni */
#if defined(_MSC_VER) && defined(DEBUG)
void libxmp_msvc_dbgprint(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	vprintf(format, argptr);
	printf("\n");
	va_end(argptr);
}
#endif
