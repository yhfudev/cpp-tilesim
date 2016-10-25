CC = @CXX@

bin_PROGRAMS=tscli
tscli_SOURCES = \
    tscli.c \
    $(NULL)

tscli_DEPENDENCIES=$(top_builddir)/src/libtilesim.a

tscli_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    $(NULL)
tscli_LDADD=-ltilesim -lxml2 -lz -ligraph $(LIBXML_LDFLAGS)

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
#AM_LDADD = -ldbi
#tscli_LDADD += -ldbi
endif

if USE_PRESENTATION
DEFS+= -DUSE_PRESENTATION=1
endif

if USE_THREEDIMENTION
DEFS+= -DUSE_THREEDIMENTION=1
endif
