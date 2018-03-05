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


/* implementation of slisp environment */
#include "slisp_env.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

slisp_env_t *enter_scope(slisp_env_t *cur, int numvars) {
  slisp_env_t *n;

  n = (slisp_env_t *)malloc(sizeof(slisp_env_t));
  assert(n != NULL);
  
  if (cur == NULL) {
    n->depth = 0;
    n->parent = NULL;
  } else {
    n->parent = cur;
    n->depth = cur->depth + 1;
  }

  assert(numvars >= 0);

  n->numvars = n->numfree = numvars;

  if (numvars > 0) {
    n->varnames = (char **)calloc(numvars,sizeof(char *));
    assert(n->varnames != NULL);

    n->bindings = (sexp_t **)calloc(numvars,sizeof(sexp_t *));
    assert(n->bindings != NULL);
  }

  n->body = NULL;

  return n;
}

slisp_env_t *exit_scope(slisp_env_t *cur) {
  slisp_env_t *p;

  assert(cur != NULL);
  p = cur->parent;

  /** put code to free contents of struct here */

  free(cur);

  return p;
}

slisp_env_t *add_varname(char *name, slisp_env_t *cur) {
  int idx, slen;

  assert(cur != NULL);
  assert(name != NULL);

  /* index that this value goes */
  idx = cur->numvars - (cur->numvars - cur->numfree);
  slen = strlen(name);

  cur->varnames[idx] = (char *)malloc(sizeof(char)*(slen + 1));
  strcpy(cur->varnames[idx],name);
  cur->varnames[slen] = '\0';

  return cur;
}

slisp_env_t *bind_variable(sexp_t *sx, slisp_env_t *cur) {
  int idx;

  assert(sx != NULL);
  assert(cur != NULL);

  if (cur->numfree == 0) {
    fprintf(stderr,"ERROR: attempting to bind a variable in an env with no free variables!\n");
    return cur;
  }

  idx = cur->numvars - (cur->numvars - cur->numfree);
  
  cur->bindings[idx] = sx;
  cur->numfree--;

  return cur;
}

slisp_env_t *set_body(sexp_t *b, slisp_env_t *cur) {
  assert(b != NULL);
  assert(cur != NULL);
  
  cur->body = b;

  return cur;
}

sexp_t *lookup_binding(char *name, slisp_env_t *cur) {
  int i;

  assert(name != NULL);
  assert(cur != NULL);

  for (i = 0; i < cur->numvars; i++)
    if (strcmp(name,cur->varnames[i]) == 0) 
      return cur->bindings[i];

  /* here means not found.  try parent. */
  if (cur->parent != NULL)
    return lookup_binding(name,cur->parent);

  return NULL;
}
