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
 * utility functions and other relevant stuff for making slisp work
 */
#ifndef __SLISP_UTIL_H__
#define __SLISP_UTIL_H__

#include "sexp.h"

/**
 * tokens for operations 
 */
typedef enum {
  /* binary logical operations */
  SL_EQ, SL_GT, SL_LT, SL_NE, SL_GEQ, SL_LEQ,

  /* unary logical operations */
  SL_NOT,

  /* mathematical operations, binary */
  SL_PLUS, SL_MINUS, SL_MULT, SL_DIVIDE, SL_EXP, 

  /* mathematical operations, unary */
  SL_SQRT,                           

  /* list construction/separation */
  SL_CONS, SL_CDR, SL_CAR,

  /* function application over lists */
  SL_FOLD, SL_MAP,

  /* list sorting */
  SL_SORT,

  /* conditional */
  SL_IF,

  /* lambda */
  SL_LAMBDA,

  /* UNKNOWN : ERROR */
  SL_UNKNOWN                         
} slisp_op_t;

/*
 * types for expression elements
 */
typedef enum {
  SL_INT, 
  SL_FLOAT, 
  SL_STRING, 
  SL_SEXP, 
  SL_INVALID
} slisp_val_t;

#ifdef __cplusplus
extern "C" {
#endif

  sexp_t      *deep_copy_sexp(sexp_t *sx);
  slisp_op_t   tokenize(sexp_t *sx);
  slisp_val_t  derive_type(sexp_t *sx);

#ifdef __cplusplus
}
#endif

#endif /* __SLISP_UTIL_H__ */
