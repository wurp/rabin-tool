CC = @CC@
CPLUS = @CXX@
CPPFLAGE = -I.
CFLAGS = $(CPPFLAGE) $(INCLS)

# Build a libtool library, librabinpoly.a for installation in libdir
lib_LIBRARIES = librabinpoly.a
librabinpoly_a_SOURCES = rabinpoly.C msb.C
#lib_LTLIBRARIES = librabinpoly.a
#librabinpoly_la_SOURCES = rabinpoly.C msb.C
#librabinpoly_la_LDFLAGS = -version-info 1:0:0

include_HEADERS = rabinpoly.h
noinst_HEADERS = msb.h

rabinpoly.o: rabinpoly.h rabinpoly.C
	$(CPLUS) $(CFLAGS) -c rabinpoly.C

msb.o: msb.h msb.C
	$(CPLUS) $(CFLAGS) -c msb.C

CLEANFILES = core *.core *~ 
MAINTAINERCLEANFILES = Makefile.in

