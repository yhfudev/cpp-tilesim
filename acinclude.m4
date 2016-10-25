#######################################################################
#
# tilesim - Self-Assembly Simulate System
# $Id$
# Copyright (C) 2008 - 2009, yhfu
# This software is released under the GPL 3 Licence
#
# acinclude.m4 - Build system scripts.
#
#######################################################################

#################################
# Check there is a C++ compiler #
#################################

AC_DEFUN([CHECK_CPP_COMPILER],
[
	if test "$ac_cv_prog_cxx_g" = no; then
		AC_MSG_ERROR([could not find a suitable C++ compiler to build pgAdmin])
	fi
])

###########################################
# Check if we can use precompiled headers #
###########################################

AC_DEFUN([CHECK_PRECOMP_HEADERS],
[
	AC_MSG_CHECKING([whether to use precompiled headers])
	USE_PRECOMP=""
	AC_ARG_ENABLE(precomp, [  --enable-precomp        Use precompiled headers], USE_PRECOMP="$enableval")

	if test -z "$USE_PRECOMP"; then
		if test "X$GCC" = Xyes; then
			if gcc_version=`$CC -dumpversion` > /dev/null 2>&1; then
				major=`echo $gcc_version | cut -d. -f1`
				minor=`echo $gcc_version | sed "s/@<:@-,a-z,A-Z@:>@.*//" | cut -d. -f2`
				if test -z "$major" || test -z "$minor"; then
					USE_PRECOMP=no
				elif test "$major" -ge 4; then
					USE_PRECOMP=yes
				elif test "$major" -ge 3 && test "$minor" -ge 4; then
					USE_PRECOMP=yes
				else
					USE_PRECOMP=no
				fi
			else
				USE_PRECOMP=no
			fi
		else
			USE_PRECOMP=no
		fi
	fi
	if test "x$USE_PRECOMP" = "xyes"; then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
])
AC_SUBST(USE_PRECOMP)

#################################
# Check this is SUN compiler #
#################################

AC_DEFUN([CHECK_SUN_COMPILER],
[
       $CC -V 2>test.txt
       SUN_STR=`head -1 test.txt |cut -f2 -d" "`
       rm -rf test.txt
       if test "$SUN_STR" = "Sun"; then
               SUN_CC_COMPILER=yes 
       fi

       $CXX -V 2>test.txt
       SUN_STR=`head -1 test.txt |cut -f2 -d" "`
       rm -rf test.txt
       if test "$SUN_STR" = "Sun"; then
               SUN_CXX_COMPILER=yes
       fi

])

#############################
# Override wxWidgets version #
#############################
AC_DEFUN([SET_WX_VERSION],
[
	AC_ARG_WITH(wx-version, [  --with-wx-version=<version number>  the wxWidgets version in major.minor format (default: 2.8)],
	[
		if test "$withval" = yes; then
			AC_MSG_ERROR([you must specify a version number when using --with-wx-version=<version number>])
		else
			if test -z "$withval"; then
				AC_MSG_ERROR([you must specify a version number when using --with-wx-version=<version number>])
			else
				WX_VERSION="$withval"
			fi
		fi
	],
	[
	    WX_VERSION=`${WX_CONFIG} --version`
	    if test -f "${WX_CONFIG}"
	    then
            changequote(<<. >>)dnl
            WX_MAJOR=`expr ${WX_VERSION} : '\([0-9]*\)'`
            WX_MINOR=`expr ${WX_VERSION} : '[0-9]*\.\([0-9]*\)'`
            changequote([, ])dnl
            WX_VERSION=${WX_MAJOR}.${WX_MINOR}
	    else
		    WX_VERSION="2.8"
		fi
	])
])

####################
# Locate wxWidgets #
####################
AC_DEFUN([LOCATE_WXWIDGETS],
[
	AC_ARG_WITH(wx, [  --with-wx=DIR	   root directory for wxWidgets installation],
	[
		if test "$withval" != no
		then
			WX_HOME="$withval"
						if test ! -f "${WX_HOME}/bin/wx-config"
						then
								AC_MSG_ERROR([Could not find your wxWidgets installation in ${WX_HOME}])
						fi

		fi
		WX_CONFIG=${WX_HOME}/bin/wx-config
	], 
	[
		WX_HOME=${prefix}
		if test ! -f "${WX_HOME}/bin/wx-config"
		then
		WX_HOME=/usr/local/wx2
		if test ! -f "${WX_HOME}/bin/wx-config"
		then
			WX_HOME=/usr/local
			if test ! -f "${WX_HOME}/bin/wx-config"
			then
				WX_HOME=/usr
				if test ! -f "${WX_HOME}/bin/wx-config"
				then
                    # Search the path
				    AC_PATH_PROGS(WX_CONFIG, wx-config)
                    if test ! -f "${WX_CONFIG}"
				    then
                        AC_MSG_ERROR([Could not find your wxWidgets installation. You might need to use the --with-wx=DIR configure option])
                    else
					   WX_HOME=`${WX_CONFIG} --prefix`
                    fi
				fi
			fi
		fi
		fi
		WX_CONFIG=${WX_HOME}/bin/wx-config
	])
])

###########################
# Check wxWidgets version #
###########################
AC_DEFUN([CHECK_WXWIDGETS],
[
        AC_MSG_CHECKING(wxWidgets version)
        TMP_WX_VERSION=`${WX_CONFIG} --version 2> /dev/null`
        if test "$TMP_WX_VERSION" = ""
        then
                 AC_MSG_RESULT(failed)
                 AC_MSG_ERROR([The version of wxWidgets required (${WX_VERSION}) is not supported by the installations in ${WX_HOME}.])
        fi
        changequote(<<. >>)dnl
        WX_MAJOR=`expr ${TMP_WX_VERSION} : '\([0-9]*\)'`
        WX_MINOR=`expr ${TMP_WX_VERSION} : '[0-9]*\.\([0-9]*\)'`
        changequote([, ])dnl
        if test "$WX_MAJOR" -lt 2; then
                AC_MSG_ERROR([wxWidgets 2.8.0 or newer is required to build pgAdmin])
        fi
        if test "$WX_MAJOR" -eq 2 && test "$WX_MINOR" -lt 8; then
                AC_MSG_ERROR([wxWidgets 2.8.0 or newer is required to build pgAdmin])
        fi
        AC_MSG_RESULT(ok)
])


#####################
# Locate libxml	#
#####################
AC_DEFUN([LOCATE_LIBXML2],
[
	AC_ARG_WITH(libxml2, [  --with-libxml2=DIR  root directory for libxml2 installation],
	[
	  if test "$withval" != no
	  then
		 XML2_HOME="$withval"
		 if test ! -f "${XML2_HOME}/bin/xml2-config"
		 then
			AC_MSG_ERROR([Could not find your libxml2 installation in ${XML2_HOME}])
		 fi
	  fi
	  XML2_CONFIG=${XML2_HOME}/bin/xml2-config
   ],
   [
	  XML2_HOME=${prefix}
	  if test ! -f "${XML2_HOME}/bin/xml2-config"
	  then
	  XML2_HOME=/usr/local
	  if test ! -f "${XML2_HOME}/bin/xml2-config"
	  then
		  XML2_HOME=/usr
		  if test ! -f "${XML2_HOME}/bin/xml2-config"
		  then
			  XML2_HOME=/mingw
			  if test ! -f "${XML2_HOME}/bin/xml2-config"
			  then
                  # Search the path
				  AC_PATH_PROGS(XML2_CONFIG, xml2-config)
                  if test ! -f "${XML2_CONFIG}"
				  then
                      AC_MSG_ERROR([Could not find your libxml2 installation. You might need to use the --with-libxml2=DIR configure option])
                  else
					  XML2_HOME=`${XML2_CONFIG} --prefix`
                  fi
			  fi
		  fi
	  fi
	  fi
	  XML2_CONFIG=${XML2_HOME}/bin/xml2-config
   ])
])

#########################
# Check libxml2 version #
#########################
AC_DEFUN([CHECK_LIBXML2],
[
	AC_MSG_CHECKING(libxml2 version)
	XML2_VERSION=`${XML2_CONFIG} --version`
	changequote(<<. >>)dnl
	XML2_MAJOR=`expr ${XML2_VERSION} : '\([0-9]*\)'`
	XML2_MINOR=`expr ${XML2_VERSION} : '[0-9]*\.\([0-9]*\)'`
	XML2_REVISION=`expr ${XML2_VERSION} : '[0-9]*\.[0-9]*\.\([0-9]*\)'`
	changequote([, ])dnl
	if test "$XML2_MAJOR" -lt 2; then
		AC_MSG_ERROR([libxml2 2.6.18 or newer is required to build pgAdmin])
	fi
	if test "$XML2_MAJOR" -eq 2 && test "$XML2_MINOR" -lt 6; then
		AC_MSG_ERROR([libxml2 2.6.18 or newer is required to build pgAdmin])
	fi
	if test "$XML2_MAJOR" -eq 2 && test "$XML2_MINOR" -eq 6 && test "$XML2_REVISION" -lt 18; then
		AC_MSG_ERROR([libxml2 2.6.18 or newer is required to build pgAdmin])
	fi
	AC_MSG_RESULT(ok)
])


#####################
# Locate libxslt    #
#####################
AC_DEFUN([LOCATE_LIBXSLT],
[
	AC_ARG_WITH(libxslt, [  --with-libxslt=DIR  root directory for libxslt installation],
	[
	  if test "$withval" != no
	  then
		 XSLT_HOME="$withval"
		 if test ! -f "${XSLT_HOME}/bin/xslt-config"
		 then
			AC_MSG_ERROR([Could not find your libxslt installation in ${XSLT_HOME}])
		 fi
	  fi
	  XSLT_CONFIG=${XSLT_HOME}/bin/xslt-config
   ],
   [
	  XSLT_HOME=${prefix}
	  if test ! -f "${XSLT_HOME}/bin/xslt-config"
	  then
	  XSLT_HOME=/usr/local
	  if test ! -f "${XSLT_HOME}/bin/xslt-config"
	  then

		  XSLT_HOME=/usr
		  if test ! -f "${XSLT_HOME}/bin/xslt-config"
		  then
			  XSLT_HOME=/mingw
			  if test ! -f "${XSLT_HOME}/bin/xslt-config"
			  then
                  # Search the path
				  AC_PATH_PROGS(XSLT_CONFIG, xslt-config)
                  if test ! -f "${XSLT_CONFIG}"
				  then
                      AC_MSG_ERROR([Could not find your libxslt installation. You might need to use the --with-libxslt=DIR configure option])
                  else
					  XSLT_HOME=`${XSLT_CONFIG} --prefix`
                  fi
			  fi
		  fi
	  fi
	  fi
	  XSLT_CONFIG=${XSLT_HOME}/bin/xslt-config
   ])
])

###########################
# Debug build of SSS      #
###########################
AC_DEFUN([ENABLE_DEBUG],
[
	AC_ARG_ENABLE(debug, [  --enable-debug	   build a debug version of SSS],
	[
		if test "$enableval" = yes
		then
			BUILD_DEBUG=yes
		else
			BUILD_DEBUG=no
		fi
	],
	[
		BUILD_DEBUG=no
	])
])
AC_SUBST(BUILD_DEBUG)

############################
# Static build of SSS      #
############################
AC_DEFUN([ENABLE_STATIC],
[
	AC_ARG_ENABLE(static, [  --enable-static	  build a statically linked version of SSS     ],
	[
		if test "$enableval" = yes
		then
			BUILD_STATIC=yes
			WX_STATIC="--static=yes"
		else
			BUILD_STATIC=no
			WX_STATIC="--static=no"
		fi
	],
	[
		BUILD_STATIC=no
		WX_STATIC="--static=no"
	])
])

##########################
# Build a Mac App Bundle #
##########################
AC_DEFUN([ENABLE_APPBUNDLE],
[
	AC_ARG_ENABLE(appbundle, [  --enable-appbundle   Build a Mac OS X appbundle],
	[
		if test "$enableval" = yes
		then
			BUILD_APPBUNDLE=yes
			prefix=$(pwd)/tmp
			bundledir="$(pwd)/SSS     .app"
			bindir="$bundledir/Contents/MacOS"
			debuggerbindir="$bundledir/Contents/Resources/Debugger.app/Contents/MacOS"
			datadir="$bundledir/Contents/SharedSupport"
			AC_SUBST(bundledir)
			AC_SUBST(debuggerbindir)
		else
			BUILD_APPBUNDLE=no
		fi
	],
	[
		BUILD_APPBUNDLE=no
	])
])

################################################
# Check for wxWidgets libraries and headers	   #
################################################
AC_DEFUN([SETUP_WXWIDGETS],
[
	if test -n "${WX_HOME}"
	then
		LDFLAGS="$LDFLAGS -L${WX_HOME}/lib"
		WX_OLD_LDFLAGS="$LDFLAGS"
		WX_OLD_CPPFLAGS="$CPPFLAGS"
	
		AC_MSG_CHECKING(wxWidgets in ${WX_HOME})
		if test "$BUILD_DEBUG" = yes
		then
			WX_NEW_CPPFLAGS=`${WX_CONFIG} --cppflags --unicode=yes --debug=yes --version=${WX_VERSION} 2> /dev/null`
			CPPFLAGS="$CPPFLAGS $WX_NEW_CPPFLAGS -g -O0"
			WX_LDADD=`${WX_CONFIG} ${WX_STATIC} --libs base,core,xml,std,stc,gl --unicode=yes --debug=yes --version=${WX_VERSION} 2> /dev/null`
			if test "$WX_NEW_CPPFLAGS" = "" -o "$WX_LDADD" = ""
			then
				WX_NEW_CPPFLAGS=`${WX_CONFIG} --cppflags --unicode=yes --debug=no --version=${WX_VERSION} 2> /dev/null`
				CPPFLAGS="$CPPFLAGS $WX_NEW_CPPFLAGS -g -O0"
				WX_LDADD=`${WX_CONFIG} ${WX_STATIC} --libs base,core,xml,std,stc,gl --unicode=yes --debug=no --version=${WX_VERSION} 2> /dev/null`
				#AC_MSG_ERROR([Your wxWidgets installation cannot support DEBUG in the selected configuration. It will switch to release version.])
			fi
		else
			WX_NEW_CPPFLAGS=`${WX_CONFIG} --cppflags --unicode=yes --debug=no --version=${WX_VERSION} 2> /dev/null`
			CPPFLAGS="$CPPFLAGS $WX_NEW_CPPFLAGS -O2 -DEMBED_XRC"
			WX_LDADD=`${WX_CONFIG} ${WX_STATIC} --libs base,core,xml,std,stc,gl --unicode=yes --debug=no --version=${WX_VERSION} 2> /dev/null`
		fi

		if test "$WX_NEW_CPPFLAGS" = "" -o "$WX_LDADD" = ""
		then
			AC_MSG_RESULT(failed)
			AC_MSG_ERROR([Your wxWidgets installation cannot support SSS in the selected configuration. This may be because it was configured without the --enable-unicode option, or the combination of dynamic/static linking and debug/non-debug libraries selected did not match any installed wxWidgets libraries.])
		else
			AC_MSG_RESULT(ok)
		fi

		case "${host}" in
			*-apple-darwin*)
				MAC_PPC=`${WX_CONFIG} --libs | grep -c "arch ppc"`
				MAC_I386=`${WX_CONFIG} --libs | grep -c "arch i386"`
                                MAC_X86_64=`${WX_CONFIG} --libs | grep -c "arch x86_64"`
				CPPFLAGS="$CPPFLAGS -no-cpp-precomp"
                                LDFLAGS="$LDFLAGS -headerpad_max_install_names"
				if test "$MAC_PPC" != "0"
				then
					CPPFLAGS="$CPPFLAGS -arch ppc"
				fi
                                if test "$MAC_I386" != "0"
                                then
                                        CPPFLAGS="$CPPFLAGS -arch i386"
                                fi
                                if test "$MAC_X86_64" != "0"
                                then
                                        CPPFLAGS="$CPPFLAGS -arch x86_64"
                                fi
				;;
			*solaris*)
				LDFLAGS="$LDFLAGS -lnsl"
				;;
			*)
				;;
		esac
	fi
])
AC_SUBST(WX_CONFIG)
AC_SUBST(WX_LDADD)

#########################
# Setup libxml2 headers #
#########################
AC_DEFUN([SETUP_LIBXML2],
[
	if test -n "${XML2_HOME}"
	then
		XML2_CFLAGS=`${XML2_CONFIG} --cflags`
		XML2_LIBS=`${XML2_CONFIG} --libs`
		AC_MSG_CHECKING(libxml2 in ${XML2_HOME})
		if test "${XML2_CFLAGS}" = "" -o "${XML2_LIBS}" = ""
		then
			AC_MSG_RESULT(failed)
			AC_MSG_ERROR([Your libxml2 installation does not appear to be complete])
		else
			AC_MSG_RESULT(ok)
			CPPFLAGS="$CPPFLAGS $XML2_CFLAGS"
			XML2_LDADD="${XML2_LDADD} $XML2_LIBS"
		fi
	fi
])
AC_SUBST(XML2_CONFIG)
AC_SUBST(XML2_LDADD)
	
#########################
# Setup libxslt headers #
#########################
AC_DEFUN([SETUP_LIBXSLT],
[
	if test -n "${XSLT_HOME}"
	then
		XSLT_CFLAGS=`${XSLT_CONFIG} --cflags`
		XSLT_LIBS=`${XSLT_CONFIG} --libs`
		AC_MSG_CHECKING(libxslt in ${XSLT_HOME})
		if test "${XSLT_CFLAGS}" = "" -o "${XSLT_LIBS}" = ""
		then
			AC_MSG_RESULT(failed)
			AC_MSG_ERROR([Your libxslt installation does not appear to be complete])
		else
			AC_MSG_RESULT(ok)
			CPPFLAGS="$CPPFLAGS $XSLT_CFLAGS"
			XSLT_LDADD="${XSLT_LDADD} $XSLT_LIBS"
		fi
	fi
])
AC_SUBST(XSLT_CONFIG)
AC_SUBST(XSLT_LDADD)

###########
# Cleanup #
###########
AC_DEFUN([CLEANUP],
[
	# CFLAGS/CXXFLAGS may well contain unwanted settings, so clear them.
	CFLAGS=""
	CXXFLAGS=""
])

#########################
# Configuration summary #
#########################
AC_DEFUN([SUMMARY],
[
	# Print a configuration summary
	echo
	echo "wxWidgets directory:			$WX_HOME"
	echo "wxWidgets wx-config binary:		$WX_CONFIG"
	echo "wxWidgets version:			wxWidgets "`$WX_CONFIG --version --version=$WX_VERSION`
	echo
	echo "libxml2 directory:			$XML2_HOME"
	echo "libxml2 xml2-config binary:		$XML2_CONFIG"
	echo "libxml2 version:			libxml2 "`$XML2_CONFIG --version`
	echo
	echo "libxslt directory:			$XSLT_HOME"
	echo "libxslt xslt-config binary:		$XSLT_CONFIG"
	echo "libxslt version:			libxslt "`$XSLT_CONFIG --version`
	echo
	if test "$BUILD_DEBUG" = yes
	then
		echo "Building a debug version:	Yes"
	else
		echo "Building a debug version:	No"
	fi
	if test "$BUILD_STATIC" = yes
	then
		echo "Statically linking software:		Yes"
	else
		echo "Statically linking software:		No"
	fi
	if test "$BUILD_APPBUNDLE" = yes
	then
		echo "Building a Mac OS X appbundle:		Yes"
	else
		echo "Building a Mac OS X appbundle:		No"
	fi
	echo

	echo "Software configuration is now complete. You can now compile and"
	echo "install it by using 'make; make install'."
	echo
])
