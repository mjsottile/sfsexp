CFLAGS = $(SFSEXP_CFLAGS)
CPPFLAGS = -I../src $(SFSEXP_CPPFLAGS)
LDFLAGS =
EXTRA_DIST = testrc vis.in sexps.in edgelist.dat conttest

noinst_PROGRAMS = binmode callbacks continuations packunpack paultest rcfile sexpvis simple_interp
noinst_HEADERS = ../src/sexp.h
LDADD = ../src/libsexp.la
binmode_SOURCES = binmode.c ../src/sexp.h
callbacks_SOURCES = callbacks.c ../src/sexp.h
continuations_SOURCES = continuations.c ../src/sexp.h
packunpack_SOURCES = packunpack.c ../src/sexp.h
paultest_SOURCES = paultest.c ../src/sexp.h
rcfile_SOURCES = rcfile.c ../src/sexp.h ../src/sexp_ops.h
sexpvis_SOURCES = sexpvis.c ../src/sexp.h ../src/sexp_vis.h
simple_interp_SOURCES = simple_interp.c ../src/sexp.h ../src/sexp_ops.h

