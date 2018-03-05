/**

SFSEXP: Small, Fast S-Expression Library version 1.0
Written by Matthew Sottile (matt@lanl.gov)

Copyright (2003-2006). The Regents of the University of California. This
material was produced under U.S. Government contract W-7405-ENG-36 for Los
Alamos National Laboratory, which is operated by the University of
California for the U.S. Department of Energy. The U.S. Government has rights
to use, reproduce, and distribute this software. NEITHER THE GOVERNMENT NOR
THE UNIVERSITY MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY
LIABILITY FOR THE USE OF THIS SOFTWARE. If software is modified to produce
derivative works, such modified software should be clearly marked, so as not
to confuse it with the version available from LANL.

Additionally, this library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, U SA

LA-CC-04-094

**/

/**
 * testing harness driver for slisp - not intended for use outside
 * development of slisp itself (and possibly as an educational aid for
 * users who wish to embed SLISP in their own code.)
 *
 * -mjs 8.2003
 */
#include "slisp.h"
#include "sexp.h"
#include "slisp_memman.h"
#include <stdio.h>
#ifndef WIN32
# include <unistd.h>
#else
# define ssize_t int
# include <io.h>
# include <sys/types.h>
#endif
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  char buf[BUFSIZ], outbuf[BUFSIZ];
  sexp_t *in, *out;
  sexp_iowrap_t *iow;
  int fd;

  reset_memory_counters();

  fd = open("test_expressions",O_RDONLY);
  iow = init_iowrap(fd);

  in = read_one_sexp(iow);
  while (in != NULL) {

    print_sexp(buf,BUFSIZ,in);
    out = slisp_eval(in);
    if (out != NULL) {
      print_sexp(outbuf,BUFSIZ,out);
      printf("%s ==> %s\n",buf,outbuf);
    } else {
      printf("%s ==> ERROR\n",buf);
    }

    destroy_sexp(in);
    if (out != NULL) {
      destroy_sexp(out);
      FREE_CHECKPOINT(out);
    }

    report_memory_counters();
    reset_memory_counters();

    in = read_one_sexp(iow);
  }

  destroy_iowrap(iow);

  exit(EXIT_SUCCESS);
}
