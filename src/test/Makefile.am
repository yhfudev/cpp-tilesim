CC = @CXX@

bin_PROGRAMS=test3d
test3d_SOURCES = \
    benchmrk.c \
    test3d.c \
    $(NULL)

test3d_DEPENDENCIES=$(top_builddir)/src/libtilesim.a

test3d_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    $(NULL)
test3d_LDADD=-ltilesim -lxml2 -lz -ligraph $(LIBXML_LDFLAGS)

INCLUDES= \
    -I$(prefix)/include/ \
    -I$(prefix)/include/libxml2/ \
    -I$(prefix)/../include/libxml2/ \
    -I$(top_builddir)/ \
    -I$(top_srcdir)/src/core/ \
    -I$(top_srcdir)/src/display/ \
    $(NULL)

DEFS+= $(LIBXML_CFLAGS)

if DEBUG
#DEFS+= -DMEMWATCH=1
DEFS+= -DDEBUG=1

AM_CFLAGS=-pg -g -Wall
AM_CXXFLAGS=-pg -g -Wall
AM_LDFLAGS=-pg -g -Wall
else
AM_CFLAGS=-pg -g -Wall
AM_CXXFLAGS=-pg -g -Wall
AM_LDFLAGS=-pg -g -Wall
endif

if USE_DBRECORD
DEFS+= -DUSE_DBRECORD=1
#AM_LDADD+= -ldbi
#tscli_LDADD += -ldbi
endif

if USE_PRESENTATION
DEFS+= -DUSE_PRESENTATION=1
endif

if USE_THREEDIMENTION
DEFS+= -DUSE_THREEDIMENTION=1
endif
