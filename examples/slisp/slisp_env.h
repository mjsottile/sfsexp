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
 * implementation of environments for SLISP interpreter.  necessary for
 * handling lambda.
 *
 * -mjs 8/2003
 */
#ifndef __SLISP_ENV_H__
#define __SLISP_ENV_H__

#include "sexp.h"

typedef struct slisp_env_struct {
  /* depth this environment is nested */
  int depth;

  /* number of free variables */
  int numfree;

  /* number of total variables */
  int numvars;

  /* names of variables, in reverse order. positions numvars-numfree ->
   * numvars-1 are bound variables.
   */
  char **varnames;

  /* bindings for bound variables.  each binding matches the name in
   * position.
   */
  sexp_t **bindings;

  /* body of the expression */
  sexp_t *body;

  /* environment containing this one */
  struct slisp_env_struct *parent;
} slisp_env_t;

#ifdef __cplusplus
extern "C" {
#endif

  slisp_env_t *enter_scope(slisp_env_t *cur, int numvars);
  slisp_env_t *exit_scope(slisp_env_t *cur);
  slisp_env_t *add_varname(char *name, slisp_env_t *cur);
  slisp_env_t *bind_variable(sexp_t *sx, slisp_env_t *cur);
  slisp_env_t *set_body(sexp_t *b, slisp_env_t *cur);
  sexp_t *lookup_binding(char *name, slisp_env_t *cur);

#ifdef __cplusplus
}
#endif

#endif /* __SLISP_ENV_H__ */
