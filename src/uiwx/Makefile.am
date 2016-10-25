
bin_PROGRAMS=tswx
tswx_SOURCES = \
    ../gl2ps/gl2ps.c \
    tswx.cpp \
    $(NULL)

tswx_DEPENDENCIES=$(top_builddir)/src/libtilesim.a

#WXCFLAGS=`PATH=$(prefix)/bin/:$(PATH) wx-config --cflags`
#WXCPPFLAGS=`PATH=$(prefix)/bin/:$(PATH) wx-config --cppflags`
WXLIB=$(WX_LDADD)

DEFS+= \
    $(WXCFLAGS) \
    $(WXCPPFLAGS) \
    $(NULL)

INCLUDES= \
    -I$(prefix)/include/ \
    -I$(prefix)/include/libxml2/ \
    -I$(prefix)/../include/libxml2/ \
    -I$(top_builddir)/ \
    -I$(top_srcdir)/src/core/ \
    -I$(top_srcdir)/src/display/ \
    -I$(top_srcdir)/src/gl2ps/ \
    $(NULL)

tswx_CXXFLAGS= \
    -DGLUT_DISABLE_ATEXIT_HACK=1

tswx_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    -L$(top_builddir)/src/libAntTweakBar/src/ \
    $(NULL)

tswx_LDADD=-ltilesim -lxml2 -lz -ligraph -lfreetype -lAntTweakBar $(WXLIB) \
  $(GL_LIBS) $(X_LIBS) $(LIBXXF86VM) $(LIBXI) $(LIBXML_LDFLAGS) #-lglut -lGLU -lGL

DEFS+= $(LIBXML_CFLAGS)

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

if USE_DBRECORD
DEFS+= -DUSE_DBRECORD=1
tswx_LDADD+= -ldbi
#tswx_LDADD += -ldbi
endif

if USE_PRESENTATION
DEFS+= -DUSE_PRESENTATION=1
endif

if USE_THREEDIMENTION
DEFS+= -DUSE_THREEDIMENTION=1
endif
