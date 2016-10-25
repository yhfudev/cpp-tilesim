CC = @CXX@

noinst_LIBRARIES=libtilesim.a
libtilesim_a_SOURCES= \
    core/cubeface.c \
    core/colortab.c \
    core/datasql.c \
    core/pfdebug.c \
    core/pfrandom.c \
    core/pfsort.c \
    core/pfutils.c \
    core/cstrutils.c \
    core/tilestruct.c \
    display/glutfont.c \
    display/tileog.c \
    display/tilepresent.c \
    display/ft2wrap.cpp \
    core/memwatch.c \
    core/tilestruct2d.c \
    core/bitset.c \
    core/tilesimatam.c \
    core/tilesimktam.c \
    $(NULL)

#DISTCLEANFILES = core/tsconfig.h

INCLUDES= \
    -I$(prefix)/include/ \
    -I$(prefix)/include/libxml2/ \
    -I$(prefix)/../include/libxml2/ \
    `$(DESTDIR)$(bindir)/freetype-config --cflags` \
    -I$(prefix)/include/freetype2/ \
    -I$(prefix)/../include/freetype2/ \
    -I$(top_srcdir)/src/core/ \
    -I$(top_srcdir)/src/display/ \
    -I$(top_srcdir)/src/libAntTweakBar/include/ \
    $(NULL)

#LDADD=-ltilesim -lxslt -lxml2 $(LIBXML2_LDFLAGS)

DEFS+= $(LIBXML2_CFLAGS)

if DEBUG
#DEFS+= -DMEMWATCH=1
DEFS+= -DDEBUG=1

AM_CFLAGS=-g -Wall -gdwarf-2
AM_CXXFLAGS=-g -Wall -gdwarf-2
AM_LDFLAGS=-g -Wall -gdwarf-2
else
AM_CFLAGS=-O2 -Wall
AM_CXXFLAGS=-O2 -Wall
AM_LDFLAGS=-Wall
endif

DEFS+= -DGLUT_DISABLE_ATEXIT_HACK=1
if USE_DBRECORD
DEFS+= -DUSE_DBRECORD=1
endif

if USE_PRESENTATION
DEFS+= -DUSE_PRESENTATION=1
endif

if USE_THREEDIMENTION
DEFS+= -DUSE_THREEDIMENTION=1
endif

SUBDIRS = . libAntTweakBar uicli uiwx utils test # uiglut uifltk
