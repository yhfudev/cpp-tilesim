CC = @CXX@

bin_PROGRAMS=convzzg2s convktam
convzzg2s_SOURCES = \
    outtds.c \
    bintree.c \
    gendecbit.cpp \
    gentypeenc.c \
    gentypeconn.c \
    gentypefixed.c \
    gendec.cpp \
    genenc.cpp \
    grouptiles.c \
    zzinfo.c \
    zzconv.cpp \
    convzzg2s.c \
    $(NULL)

convzzg2s_DEPENDENCIES=$(top_builddir)/src/libtilesim.a

convzzg2s_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    $(NULL)
convzzg2s_LDADD=-ltilesim -lxml2 -lz -ligraph $(LIBXML_LDFLAGS)

convktam_SOURCES = \
    outtds.c \
    grouptiles.c \
    zzinfo.c \
    convktam.c \
    $(NULL)

convktam_DEPENDENCIES=$(top_builddir)/src/libtilesim.a

convktam_LDFLAGS= \
    -L$(prefix)/lib/ \
    -L$(top_builddir)/src/ \
    $(NULL)
convktam_LDADD=-ltilesim -lxml2 -lz -ligraph $(LIBXML_LDFLAGS)

INCLUDES= \
    -I$(prefix)/include/ \
    -I$(top_builddir)/ \
    -I$(top_srcdir)/src/core/ \
    $(NULL)

#DEFS+= $(LIBXML_CFLAGS)

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
