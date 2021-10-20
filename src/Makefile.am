CFLAGS = $(SFSEXP_CFLAGS)
CPPFLAGS = $(SFSEXP_CPPFLAGS)

lib_LTLIBRARIES = libsexp.la
include_HEADERS = sexp.h sexp_vis.h sexp_ops.h sexp_memory.h sexp_errors.h cstring.h faststack.h
libsexp_la_SOURCES = cstring.c cstring.h event_temp.c faststack.c faststack.h io.c parser.c sexp.c sexp.h sexp_memory.c sexp_memory.h sexp_errors.h sexp_ops.c sexp_ops.h sexp_vis.c sexp_vis.h
libsexp_la_LDFLAGS = -version-info 1:0:0
