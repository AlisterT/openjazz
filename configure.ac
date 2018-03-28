AC_INIT([openjazz], [0], [https://github.com/AlisterT/openjazz/issues], [openjazz], [http://www.alister.eu/jazz/oj/])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects tar-pax no-dist-gzip dist-xz])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CANONICAL_HOST
AC_PROG_CXX
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([SDL], [sdl])
PKG_CHECK_MODULES([LIBXMP], [libxmp], [AC_DEFINE(USE_XMP, [1], [Enable XMP Music])], [
	PKG_CHECK_MODULES([LIBMODPLUG], [libmodplug], [AC_DEFINE(USE_MODPLUG, [1], [Enable ModPlug Music])], [
		AC_MSG_WARN([No music library found. OpenJazz will run, but music will be silent.])
	])
])
PKG_CHECK_MODULES([ZLIB], [zlib])
AS_CASE([$host], [*-*-mingw*], [NET_LIBS="-lws2_32"])
AC_SUBST(NET_LIBS)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
