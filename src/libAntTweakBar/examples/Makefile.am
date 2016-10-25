bin_PROGRAMS=TwSimpleGLUT TwString # TwSimpleSDL TwAdvanced1 TwSimpleGLFW TwSimpleDX9 TwSimpleDX10

#TwAdvanced1_SOURCES = \
    TwAdvanced1.cpp \
    $(NULL)

#TwSimpleDX9_SOURCES = \
    TwSimpleDX9.cpp \
    $(NULL)

#TwSimpleDX10_SOURCES = \
    TwSimpleDX10.cpp \
    $(NULL)

#TwSimpleGLFW_SOURCES = \
    TwSimpleGLFW.c \
    $(NULL)

TwSimpleGLUT_SOURCES = \
    TwSimpleGLUT.c \
    $(NULL)

#TwSimpleSDL_SOURCES = \
    TwSimpleSDL.c \
    $(NULL)

TwString_SOURCES = \
    TwString.c \
    $(NULL)

#TwSimpleGLFW_DEPENDENCIES=$(top_builddir)/src/libAntTweakBar.a

TwSimpleGLUT_LDADD=-lAntTweakBar -lglut
#TwSimpleSDL_LDADD=-lAntTweakBar -lSDL
#TwSimpleGLFW_LDADD=-lAntTweakBar -lglfw
TwString_LDADD=-lAntTweakBar -lglfw
#TwAdvanced1_LDADD=-lAntTweakBar -lglfw

INCLUDES = \
    -I$(top_builddir)/ \
    -I$(top_srcdir)/include/ \
    $(X_CFLAGS) \
    $(NULL)

if DEBUG
DEFS+= -DMEMWATCH=1
DEFS+= -DDEBUG=1

AM_CFLAGS=-g -Wall -gdwarf-2
AM_CXXFLAGS=-g -Wall -gdwarf-2
AM_LDFLAGS=-g -Wall -gdwarf-2
else
AM_CFLAGS=-O2 -Wall
AM_CXXFLAGS=-O2 -Wall
AM_LDFLAGS=-Wall

endif

AM_LDFLAGS += -L$(top_builddir)/src/
