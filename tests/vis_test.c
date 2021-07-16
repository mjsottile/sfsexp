/**

SFSEXP: Small, Fast S-Expression Library version 1.3
Written by Matthew Sottile (mjsottile@gmail.com)

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

#ifndef WIN32
# include <unistd.h>
#else
# define ssize_t int
# include <io.h>
# include <sys/types.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "sexp.h"
#include "sexp_vis.h"

/**
 * read file "test_expressions", write DOT representation of the n'th s-expression 
 * as "out<n>.dot".
 */
int main(int argc, char **argv) {
  sexp_t *a;
  int fd;
  sexp_iowrap_t *iow;
  int count = 0;
  unsigned int timeout = 1;

  if (argc>1)
    timeout = atoi (argv[1]);

  /* Watchdog timer, to catch infinite loops */
  alarm(timeout);
  
  fd = open("test_expressions",O_RDONLY);

  iow = init_iowrap(fd);

  a = read_one_sexp(iow);

  while (a != NULL) {
    char fname[4096];
    sprintf (fname, "out%02d.dot", count++);
    sexp_to_dotfile(a,fname);

    destroy_sexp(a);
    a = read_one_sexp(iow);
  }

  destroy_iowrap(iow);
  sexp_cleanup();
  close(fd);

  exit(EXIT_SUCCESS);
}
