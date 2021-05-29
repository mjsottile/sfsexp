CFLAGS = $(SFSEXP_CFLAGS)
CPPFLAGS = -I../src $(SFSEXP_CPPFLAGS)
LDFLAGS =
EXTRA_DIST = test_expressions dotests.sh randsexp.pl

noinst_PROGRAMS = bug ctest ctorture error_codes partial read_and_dump readtests
LDADD = ../src/libsexp.la
bug_SOURCES = bug.c ../src/sexp.h
ctest_SOURCES = ctest.c ../src/sexp.h
ctorture_SOURCES = ctorture.c ../src/sexp.h
error_codes_SOURCES = error_codes.c ../src/sexp.h
partial_SOURCES = partial.c ../src/sexp.h
read_and_dump_SOURCES = read_and_dump.c ../src/sexp.h
readtests_SOURCES = readtests.c ../src/sexp.h
