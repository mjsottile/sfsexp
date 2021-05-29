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

#include <stdio.h>
#include "sexp.h"
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define RAWSTRING "(i am the test '(expression) \"with\" 'weird atoms)\0"

int main(int argc, char **argv) {
  sexp_t *sx;
  unsigned int len;
  pcont_t *pc = NULL;
  char outbuf[6];
  char inbuf[256];
  
  strcpy(inbuf,RAWSTRING);
  len = strlen(inbuf);
  
  pc = cparse_sexp(inbuf,len,pc);
  assert(pc != NULL);
  sx = pc->last_sexp;
  outbuf[5] = '\0';
  print_sexp(outbuf,5,sx);
  destroy_sexp(sx);
  destroy_continuation(pc);
  
  printf("%s\n",outbuf);
  printf("sexp_errno: %d\n", sexp_errno);
  sexp_cleanup();

  exit(EXIT_SUCCESS);
}
