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
 * utility routines of some use to the slisp implementation, removed from
 * the eval source file to unclutter things.  also, these routines may
 * prove useful enough in a general sense that they may be migrated into
 * the main s-expression library code base in the future.
 *
 * -mjs 8.2003
 */
#include "slisp_util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 *
 */
sexp_t *deep_copy_sexp(sexp_t *sx) {
  sexp_t *c_sx;

  if (sx == NULL) return NULL;

  c_sx = (sexp_t *)malloc(sizeof(sexp_t));
  assert(c_sx != NULL);

  c_sx->ty = sx->ty;
  c_sx->aty = sx->aty;

  if (sx->ty == SEXP_VALUE) {
    switch(sx->aty) {
    case SEXP_BASIC:
    case SEXP_SQUOTE:
    case SEXP_DQUOTE:
      c_sx->val_used = sx->val_used;
      c_sx->val_allocated = sx->val_allocated;
      c_sx->val = (char *)malloc(sizeof(char)*c_sx->val_allocated);
      assert(c_sx->val != NULL);
      memcpy(c_sx->val,sx->val,c_sx->val_used);

      c_sx->bindata = NULL;
      c_sx->binlength = 0;
      break;

    case SEXP_BINARY:
      c_sx->val_used = c_sx->val_allocated = 0;
      c_sx->val = NULL;
      c_sx->binlength = sx->binlength;
      c_sx->bindata = (char *)malloc(sizeof(char)*c_sx->binlength);

      assert(c_sx->bindata != NULL);
      memcpy(c_sx->bindata,sx->bindata,sx->binlength);
      
      break;

    default:
      fprintf(stderr,"ERROR: Unknown atom type in SEXP_VALUE element.\n");

      break;
    }
  } else {
    /* this is a list - so null out all atom data pointers, set counts
       to zero, and deal with the list/next fields.  if someone was
       trying to be clever and was hiding stuff in these fields, they're
       screwed.  :)  */
    c_sx->val = NULL;
    c_sx->val_allocated = c_sx->val_used = 0;
    c_sx->binlength = 0;
    c_sx->bindata = NULL;
    c_sx->list = deep_copy_sexp(sx->list);
    c_sx->next = deep_copy_sexp(sx->next);
  }

  return c_sx;
}

/**
 * Given a sexp_t element, return the token that it represents.
 */
slisp_op_t tokenize(sexp_t *sx) {
  if (sx->ty != SEXP_VALUE) {
    return SL_UNKNOWN;
  }

  if      (strcmp("+",sx->val) == 0) return SL_PLUS;
  else if (strcmp("-",sx->val) == 0) return SL_MINUS;
  else if (strcmp("*",sx->val) == 0) return SL_MULT;
  else if (strcmp("/",sx->val) == 0) return SL_DIVIDE;
  else if (strcmp("^",sx->val) == 0) return SL_EXP;
  else if (strcmp("=",sx->val) == 0) return SL_EQ;
  else if (strcmp(">",sx->val) == 0) return SL_GT;
  else if (strcmp("<",sx->val) == 0) return SL_LT;
  else if (strcmp("<=",sx->val) == 0) return SL_LEQ;
  else if (strcmp(">=",sx->val) == 0) return SL_GEQ;
  else if (strcmp("<>",sx->val) == 0) return SL_NE;
  else if (strcmp("if",sx->val) == 0) return SL_IF;
  else if (strcmp("not",sx->val) == 0) return SL_NOT;
  else if (strcmp("cdr",sx->val) == 0) return SL_CDR;
  else if (strcmp("car",sx->val) == 0) return SL_CAR;
  else if (strcmp("map",sx->val) == 0) return SL_MAP;
  else if (strcmp("cons",sx->val) == 0) return SL_CONS;
  else if (strcmp("fold",sx->val) == 0) return SL_FOLD;
  else if (strcmp("sort",sx->val) == 0) return SL_SORT;
  else if (strcmp("sqrt",sx->val) == 0) return SL_SQRT;
  else if (strcmp("lambda",sx->val) == 0) return SL_LAMBDA;
  
  return SL_UNKNOWN;
}

/**
 * Given an expression element, try to derive the type.
 */
slisp_val_t derive_type(sexp_t *sx) {
  slisp_val_t ty = SL_INT;
  char *p;

  if (sx->ty == SEXP_LIST) return SL_SEXP;
  p = sx->val;

  if (p == NULL) return SL_INVALID;

  /* only one minus, first character, is allowed while still remaining a
     numeric type. */
  if (p[0] == '-') p++;

  while (p[0] != '\0' && ty != SL_STRING) {
    if (p[0] == '.') {
      if (ty == SL_INT) ty = SL_FLOAT;
      else ty = SL_STRING;
    } else if (p[0] > '9'|| p[0] < '0') ty = SL_STRING;
    p++;
  }

  return ty;
}
