bin_PROGRAMS=squarecreator cubecreator bintilecreator cuberot

squarecreator_SOURCES = \
    squarecreator.cpp \
    $(NULL)

cubecreator_SOURCES = \
    ../core/cubeface.c \
    cubecreator.cpp \
    $(NULL)

bintilecreator_SOURCES = \
    bintilecreator.cpp \
    $(NULL)

cuberot_SOURCES = \
    cuberot.c \
    #../core/cubeface.c \
    $(NULL)

cuberot_DEPENDENCIES=$(top_builddir)/src/libtilesim.a
cuberot_LDADD=-ltilesim -lxml2 -lz -ligraph $(LIBXML_LDFLAGS)
cuberot_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    $(NULL)

#$(top_builddir)/src/libtilesim.a

INCLUDES = \
    -I$(prefix)/include/ \
    -I$(prefix)/include/libxml2/ \
    -I$(prefix)/../include/libxml2/ \
    -I$(top_builddir)/ \
    -I$(top_srcdir)/src/core
LDADD = -lxml2 $(LIBXML_LDFLAGS)

DEFS+= $(LIBXML_CFLAGS)

if DEBUG
#DEFS+= -DMEMWATCH=1
DEFS+= -DDEBUG=1

AM_CFLAGS=-pg -g -Wall -gdwarf-2
AM_CXXFLAGS=-pg -g -Wall -gdwarf-2
AM_LDFLAGS=-pg -g -Wall -gdwarf-2
else
AM_CFLAGS=-O2 -Wall
AM_CXXFLAGS=-O2 -Wall
AM_LDFLAGS=-Wall

endif

if USE_DBRECORD
DEFS+= -DUSE_DBRECORD=1
endif

if USE_PRESENTATION
DEFS+= -DUSE_PRESENTATION=1
endif

if USE_THREEDIMENTION
DEFS+= -DUSE_THREEDIMENTION=1
endif

SUBDIRS = . convazzg2s
