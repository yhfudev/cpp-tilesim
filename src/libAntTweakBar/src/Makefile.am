
CC=@CXX@

noinst_LIBRARIES=libAntTweakBar.a
libAntTweakBar_a_SOURCES= \
    TwColors.cpp \
    TwFonts.cpp \
    TwOpenGL.cpp \
    TwBar.cpp \
    TwMgr.cpp \
    TwPrecomp.cpp \
    LoadOGL.cpp \
    TwEventGLUT.c \
    #TwEventGLFW.c \
    #TwEventSDL.c

INCLUDES= \
    -I$(top_srcdir)/include/

DEFS = -D__PLACEMENT_NEW_INLINE
# -DHAVE_DIRECT3D=1

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
