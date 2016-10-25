bin_PROGRAMS=tsfltk
tsfltk_SOURCES = \
    tsfltk.cpp \
    $(NULL)

tsfltk_DEPENDENCIES=$(top_srcdir)/src/libtilesim.a

#FLTKCFLAGS=`fltk-config --cflags`
#FLTKCPPFLAGS=`fltk-config --cxxflags`
#FLTKLIB=`fltk-config --ldflags` #fltk-config --ldstaticflags
FLTKCFLAGS=$(shell fltk-config --cflags)
FLTKCPPFLAGS=$(shell fltk-config --cxxflags)
FLTKLIB=#$(shell fltk-config --ldflags)

DEFS+= $(LIBXML_CFLAGS)

DEFS+= \
    $(FLTKCFLAGS) \
    $(FLTKCPPFLAGS) \
    #-DUSE_DBRECORD=1 \
    #-DUSE_PRESENTATION=1 \
    $(NULL)

INCLUDES= \
    -I$(prefix)/include/ \
    -I$(prefix)/include/libxml2/ \
    -I$(prefix)/../include/libxml2/ \
    -I$(top_builddir)/ \
    -I$(top_srcdir)/src/core/ \
    -I$(top_srcdir)/src/display/ \
    $(NULL)

tsfltk_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    $(FLTKLIB) \
    $(NULL)
tsfltk_LDADD= -ltilesim -lxml2 -lz -ligraph -lfltk -lfreetype -lAntTweakBar $(GL_LIBS) $(X_LIBS) $(LIBXXF86VM) $(LIBXI) $(LIBXML_LDFLAGS) #-lGLU -lGL

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
tsfltk_LDADD
AM_LDADD = -ldbi
#tsfltk_LDADD += -ldbi
endif

if USE_PRESENTATION
DEFS+= -DUSE_PRESENTATION=1
endif

if USE_THREEDIMENTION
DEFS+= -DUSE_THREEDIMENTION=1
endif
