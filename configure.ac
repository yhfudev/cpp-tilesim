#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Check for WIN32, without needing all the config.guess bulkiness.
# http://angband.pl/viewvc/termrec/branches/vt100-events/ac_check_win32.m4
AC_DEFUN([AC_CHECK_WIN32], [
AC_CACHE_CHECK([for Win32], [ac_cv_is_win32], [
ac_check_win32_save_LIBS=$LIBS
LIBS=-lgdi32
AC_LINK_IFELSE([#include <windows.h>
int main(){TextOutW(0,0,0,0,0);return 0;}], [ac_cv_is_win32=yes], [ac_cv_is_win32=no])
LIBS=$ac_check_win32_save_LIBS
])
if test $ac_cv_is_win32 = yes; then
  AC_DEFINE([IS_WIN32], 1, [Define if you're a Bill worshipper.])
fi
])

AC_PREREQ(2.61)
AC_INIT(Supertile-Simulation-System, 1.0, yhfudev@gmail.com)
AC_CONFIG_SRCDIR([src/uiwx/tswx.cpp])
AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(tilesim,1.0)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
SED=sed

dnl
dnl Some packages need to be checked against version numbers so we
dnl define a function here for later use
dnl
AC_DEFUN([VERSION_TO_NUMBER],
[`$1 | sed -e 's/libxml //' | awk 'BEGIN { FS = "."; } { printf "%d", ([$]1 * 1000 + [$]2) * 1000 + [$]3;}'`])

dnl
dnl The following new parameters were added to offer
dnl the ability to specify the location of the libxml
dnl library during linking and compilation.
dnl
dnl original work - Mathieu Lacage 30/03/2000
dnl some tweaking - David Härdeman 30/10/2001
dnl

LIBXML_CONFIG_PREFIX=""
LIBXML_SRC=""

AC_ARG_WITH(libxml-prefix,
        [  --with-libxml-prefix=[PFX]        Specify location of libxml config],
    LIBXML_CONFIG_PREFIX=$withval
)

AC_ARG_WITH(libxml-include-prefix,
        [  --with-libxml-include-prefix=[PFX]    Specify location of libxml headers],
        LIBXML_CFLAGS="-I$withval"
)

AC_ARG_WITH(libxml-libs-prefix,
        [  --with-libxml-libs-prefix=[PFX]    Specify location of libxml libs],
        LIBXML_LIBS="-L$withval"
)

AC_ARG_WITH(libxml-src,
    [  --with-libxml-src=[DIR]              For libxml thats not installed yet (sets all three above)],
    LIBXML_SRC="$withval"
)
AC_SUBST(LIBXML_SRC)

dnl
dnl where is xml2-config
dnl

AC_SUBST(LIBXML_REQUIRED_VERSION)
AC_MSG_CHECKING(for libxml libraries >= $LIBXML_REQUIRED_VERSION) 
if test "x$LIBXML_CONFIG_PREFIX" != "x"
then
    XML_CONFIG=${LIBXML_CONFIG_PREFIX}/bin/xml2-config
else
    XML_CONFIG=xml2-config
fi

dnl
dnl imported from libxml2, c.f. #77827 
dnl
if test "${GCC}" != "yes" ; then
    case "${host}" in
          *-*-hpux* )
           CFLAGS="${CFLAGS} -Wp,-H30000"
           ;;
          *-dec-osf* )
               CFLAGS="${CFLAGS} -ieee"
               ;;
    esac
else
    CFLAGS="${CFLAGS} -Wall"
    case "${host}" in
          alpha*-*-linux* )
           CFLAGS="${CFLAGS} -mieee"
           ;;
      alpha*-*-osf* )
           CFLAGS="${CFLAGS} -mieee"
           ;;
    esac
fi

dnl
dnl Override other variables if LIBXML_SRC is set
dnl

if test "x$LIBXML_SRC" != "x"
then
    CWD=`pwd`
    if cd $LIBXML_SRC
    then
        SRC_DIR=`pwd`
        XML_CONFIG=${SRC_DIR}/xml2-config
        LIBXML_CFLAGS="-I${SRC_DIR}/include"
        LIBXML_LIBS="-L${SRC_DIR}"
        cd $CWD
    else
        AC_MSG_ERROR([libxml source dir not found (${LIBXML_SRC}), typo?])
    fi
fi

dnl
dnl make sure xml2-config is executable,
dnl test version and init our variables
dnl

if ${XML_CONFIG} --libs print > /dev/null 2>&1
then
    XMLVERS=`$XML_CONFIG --version`
    if test VERSION_TO_NUMBER(echo $XMLVERS) -ge VERSION_TO_NUMBER(echo $LIBXML_REQUIRED_VERSION)
    then
        LIBXML_LIBS="$LIBXML_LIBS `$XML_CONFIG --libs`"
        LIBXML_CFLAGS="$LIBXML_CFLAGS `$XML_CONFIG --cflags`"
        AC_MSG_RESULT($XMLVERS found)
        HAVE_LIBXML=1
    else
        AC_MSG_WARN(Version $XMLVERS found. You need at least libxml2 $LIBXML_REQUIRED_VERSION for this version of libxslt)
    fi
fi

AC_CHECK_WIN32
if test x"$ac_cv_is_win32" = xyes; then
  GL_LIBS="-lglut -lglu32 -lopengl32 -lgdi32 -lwinmm"
  LIBXML_LDFLAGS+=" -liconv -lwsock32 -lpthread"
  LIBXXF86VM=
  LIBXI=
  VERSION_INFO=
  LIBTW_EXTRA_CFLAGS="-D_WINDOWS"
else
  GL_LIBS="-lglut -lGLU -lGL -lXext -lX11"
  AC_CHECK_LIB([Xxf86vm], [XF86VidModeSwitchToMode],
               [LIBXXF86VM=-lXxf86vm], [LIBXXF86VM=],
               [$X_LIBS -lXext -lX11])
  LIBXI=-lXi
  VERSION_INFO="-version-info 12:0:9"
  LIBTW_EXTRA_CFLAGS="-D_UNIX"
fi

#LIBXML_CFLAGS="-DLIBXML_STATIC_FOR_DLL"
#LIBXML_CFLAGS="-DLIBXML_STATIC"
AC_SUBST([LIBXML_LDFLAGS])
AC_SUBST([LIBXML_CFLAGS])

AC_SUBST([GL_LIBS])
AC_SUBST([LIBXXF86VM])
AC_SUBST([LIBXI])
AC_SUBST([VERSION_INFO])
AC_SUBST([LIBTW_EXTRA_CFLAGS])

# debug
ENABLE_DEBUG
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug],[Compile the debug version (default: disabled)]),
    [enable_debug=$enableval],
    [enable_debug=no])
AM_CONDITIONAL([DEBUG], [test $enable_debug = "yes"])
if test "x$enable_debug" = "xyes"; then
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  changequote([,])
  dnl add -O0 only if GCC or ICC is used
  if test "$GCC" = "yes" || test "$ICC" = "yes"; then
    CFLAGS="$CFLAGS -O0"
    CXXFLAGS="$CXXFLAGS -O0"
  fi
else
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-g//g'`
  CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-g//g'`
  changequote([,])
fi

AC_ARG_ENABLE([dbrecord],
    AS_HELP_STRING([--enable-dbrecord],[Add dbrecord of the simulation (default: disable)]),
    [disable_dbrecord=$enableval],
    [disable_dbrecord=no])
AM_CONDITIONAL([USE_DBRECORD], [test $disable_dbrecord = "yes"])

AC_ARG_ENABLE([presentation],
    AS_HELP_STRING([--disable-presentation],[Disable presentation of the simulation (default: enabled)]),
    [disable_presentation=$enableval],
    [disable_presentation=yes])
AM_CONDITIONAL([USE_PRESENTATION], [test $disable_presentation = "yes"])

AC_ARG_ENABLE([threedimention],
    AS_HELP_STRING([--disable-threedimention],[Disable support for 3D (default: enabled)]),
    [disable_3d=$enableval],
    [disable_3d=yes])
AM_CONDITIONAL([USE_THREEDIMENTION], [test $disable_3d = "yes"])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT8_T

# Checks for library functions.
#AC_FUNC_MALLOC # will define malloc to rpl_malloc
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday memmove memset select])

LOCATE_WXWIDGETS
SET_WX_VERSION
CHECK_WXWIDGETS
SETUP_WXWIDGETS

LOCATE_LIBXML2
SETUP_LIBXML2
CHECK_LIBXML2

#LOCATE_LIBXSLT
#SETUP_LIBXSLT

CLEANUP

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/uicli/Makefile
                 src/uiglut/Makefile
                 src/uiwx/Makefile
                 src/test/Makefile
                 src/utils/convazzg2s/Makefile
                 src/utils/Makefile])

AC_CONFIG_SUBDIRS([src/libAntTweakBar])

AC_OUTPUT

SUMMARY
