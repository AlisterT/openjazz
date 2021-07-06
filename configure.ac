AC_INIT([openjazz], [0], [https://github.com/AlisterT/openjazz/issues], [openjazz], [http://www.alister.eu/jazz/oj/])

AC_CONFIG_AUX_DIR([builds/autotools])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects tar-pax no-dist-gzip dist-xz])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CXX
AC_PROG_CXXCPP
AM_PROG_AR
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

AC_CANONICAL_HOST
AC_C_BIGENDIAN
AC_C_RESTRICT

PKG_CHECK_MODULES([SDL], [sdl])

AS_CASE([$host],
	[*-*-mingw*], [
		NET_LIBS="-lws2_32"
		OJ_HOST="MinGW (Windows)"
		AC_CHECK_TOOLS(WINDRES, [windres])
		AS_IF([test "x$WINDRES" = "x"],
			AC_MSG_ERROR([windres is required to embed the executable icon]))
	],
	[*-haiku*], [HOST_CFLAGS="-D_BSD_SOURCE" HOST_LIBS="-lbe" NET_LIBS="-lnetwork" OJ_HOST="Haiku"],
	[*-riscos*], [
		HOST_LIBS="-static"
		OJ_HOST="RISC OS"
		AC_CHECK_PROGS(ELF2AIF, [elf2aif])
		AS_IF([test "x$ELF2AIF" = "x"],
			AC_MSG_ERROR([elf2aif is required to create the executable]))
	],
	[arm-none-linux-gnueabi], [HOST_CFLAGS="-DPANDORA -fsigned-char" HOST_LIBS="-lts" OJ_HOST="Pandora"],
	[arm-gph-linux-gnueabi], [HOST_CFLAGS="-DCAANOO -fsigned-char" OJ_HOST="GP2X Caanoo"],
	[arm-openwiz-linux-gnu], [HOST_CFLAGS="-DWIZ -fsigned-char" OJ_HOST="GP2X Wiz"],
	[arm-open2x-linux], [HOST_CFLAGS="-DGP2X -fsigned-char" OJ_HOST="GP2X"],
	[mipsel-linux*], [HOST_CFLAGS="-DDINGOO" OJ_HOST="Dingoo"],
	[armv7l-unknown-linux-gnueabihf], [HOST_CFLAGS="-DGAMESHELL" OJ_HOST="GameShell"],
	[powerpc-unknown-eabi], [
		HOST_CFLAGS="-DWII"
		NET_LIBS="-lSDL_net"
		OJ_HOST="Wii"
		AC_CHECK_PROGS(ELF2DOL, [elf2dol])
		AS_IF([test "x$ELF2DOL" = "x"],
			AC_MSG_ERROR([elf2dol is required to create the executable]))
	])
AC_SUBST(HOST_CFLAGS)
AC_SUBST(HOST_LIBS)
AC_SUBST(NET_LIBS)
AM_CONDITIONAL([OJ_WII], [test "x$OJ_HOST" = "xWii"])
AM_CONDITIONAL([OJ_RISCOS], [test "x$OJ_HOST" = "xRISC OS"])
AM_CONDITIONAL([OJ_MINGW], [test "x$OJ_HOST" = "MinGW (Windows)"])

AC_CHECK_PROGS([POD2MAN], [pod2man])
AM_CONDITIONAL([HAVE_POD2MAN], [test "x$POD2MAN" != "x"])
AS_IF([test "x$POD2MAN" = "x" && test ! -f "${srcdir}/builds/unix/OpenJazz.6"],
	AC_MSG_WARN([pod2man is required to create the manual page]))
AM_CONDITIONAL([HAVE_MANUAL], [test -f "${srcdir}/builds/unix/OpenJazz.6"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AS_IF([test "x$OJ_HOST" != "x"],[
	AS_BOX([Detected Platform: $OJ_HOST. If this is wrong, please open a bug report!], [ ])
])
