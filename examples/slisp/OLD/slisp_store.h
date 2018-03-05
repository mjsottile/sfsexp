#ifndef __SLISP_STORE_H__
#define __SLISP_STORE_H__

#include "sexp.h"

typedef struct slisp_varmap {
  char   vname[255];
  sexp_t *value;
  struct slisp_varmap *next;
} varmap_t; 

typedef struct slisp_store {
  varmap_t *vmap;
} slisp_store_t;

void dump_store(slisp_store_t *s);
void set_variable(char *name, sexp_t *sx, slisp_store_t *store);
sexp_t *get_variable(char *name, slisp_store_t *store, int max_depth);
slisp_store_t *init_store();
void destroy_varmap(varmap_t *v);
void destroy_store(slisp_store_t *s);
void enter_scope(slisp_store_t *s);
void exit_scope(slisp_store_t *s);

#endif /* __SLISP_STORE_H__ */
