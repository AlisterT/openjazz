AC_INIT([openjazz], [0], [https://github.com/AlisterT/openjazz/issues], [openjazz], [http://www.alister.eu/jazz/oj/])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects tar-pax no-dist-gzip dist-xz])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_PROG_CXX
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([libSDL], [sdl])
PKG_CHECK_MODULES([libmodplug], [libmodplug], [AC_DEFINE(USE_MODPLUG, [1], [Enable ModPlug Music])], [
	PKG_CHECK_MODULES([libxmp], [libxmp], [AC_DEFINE(USE_XMP, [1], [Enable XMP Music])])
])
PKG_CHECK_MODULES([zlib], [zlib])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
