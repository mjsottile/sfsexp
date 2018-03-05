/**
 * implementation of a primitive store for slisp
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "slisp_store.h"

/**
 * set_variable(name,sx,store)
 *
 * Bind the name to the given s-expression in the store.
 */
void set_variable(char *name, sexp_t *sx, slisp_store_t *store) {
  varmap_t *prev, *v, *tmpv;
  
  /* paranoia */
  assert(store != NULL);
  assert(sx != NULL);
  assert(name != NULL);

#ifdef _DEBUG_
  fprintf(stderr,"[STORE]: SET %s.\n",name);
#endif /* _DEBUG_ */

  prev = NULL;
  v = store->vmap;

  while (v != NULL) {
    if (strcmp(name,v->vname) == 0) {
      destroy_sexp(v->value);
      v->value = sx;
    } else {
      /* move on, remember the last varmap we visited */
      prev = v;
      v = v->next;
    }
  }

  /* at this point we know the name we're storing wasn't previously
     bound */
  
  /* allocate a varmap element */
  tmpv = (varmap_t *)malloc(sizeof(varmap_t));
  assert(tmpv != NULL);

  /* no next */
  tmpv->next = NULL;
  
  /* save the s-expression */
  tmpv->value = sx;

  /* save the name */
  strcpy(tmpv->vname,name);

  /* if prev == NULL, then the store had no variables bound, so this is
     the first.  otherwise, attach this vmap to the previous one looked at. */
  if (prev == NULL) store->vmap = tmpv;
  else prev->next = tmpv;
}

/**
 * get_variable(name,store)
 * 
 * find the named variable in the store and return it.  if it isn't bound, 
 * return NULL.
 */
sexp_t *get_variable(char *name, slisp_store_t *store, int max_depth) {
  varmap_t *v,*tv;
  sexp_t *sx = NULL;

  assert(store != NULL);
  assert(name != NULL);

#ifdef _DEBUG_
  fprintf(stderr,"[STORE]: GET %s.\n",name);
#endif /* _DEBUG_ */

  v = store->vmap;

  while (v != NULL)
    if (strcmp(name,v->vname) == 0) {
      tv = v;
      while (tv->entered_depth > max_depth && tv->shadowed != NULL) 
        tv = tv->shadowed;
      sx = copy_sexp(tv->value);
      break;
    } else 
      v = v->next;

  return sx;
}

/**
 * dump_store(s)
 * 
 * for debugging : dump the current contents of the store.
 */
void dump_store(slisp_store_t *s) {
  varmap_t *v1;
  char sxbuf[BUFSIZ];

  fprintf(stderr,"====> DUMPING STORE <====\n");

  if (s == NULL) {
    fprintf(stderr,"Store is null.\n");
    return;
  }

  v1 = s->vmap;

  while (v1 != NULL) {
    print_sexp(sxbuf,BUFSIZ,v1->value);

    fprintf(stderr,"[%s]%d :: 0x%x = \"%s\"\n",
            v1->vname,v1->entered_depth,
            (unsigned int)v1->value,sxbuf);

    v2 = v1->shadowed;
    indent = 1;
    while (v2 != NULL) {
      for(i=0;i<indent;i++) fprintf(stderr," ");
      
      print_sexp(sxbuf,BUFSIZ,v2->value);

      fprintf(stderr,"[%s]%d :: 0x%x = \"%s\"\n",
              v2->vname,v2->entered_depth,
              (unsigned int)v2->value,sxbuf);

      v2 = v2->shadowed;
      indent++;
    }

    v1 = v1->next;
  }
}

/**
 * init_store()
 *
 * create a new empty store and return it.
 */
slisp_store_t *init_store() {
  slisp_store_t *s = (slisp_store_t*)malloc(sizeof(slisp_store_t));

  assert(s != NULL);

  s->vmap = NULL;

  return s;
}

slisp_store_t *copy_store(slisp_store_t *store) {
  slisp_store_t *new_store;
  varmap_t *vm,*nvm,*prev;

  if (store == NULL) return NULL;

  new_store = (slisp_store_t *)malloc(sizeof(slisp_store_t));
  assert(new_store != NULL);

  vm = store->vmap;
  prev = NULL;

  while (vm != NULL) {
    nvm = (varmap_t *)malloc(sizeof(varmap_t));
    assert(nvm != NULL);

    strcpy(nvm->vname,vm->vname);
    nvm->value = copy_sexp(vm->value);
    nvm->next = NULL;

    if (prev != NULL) prev->next = nvm;
    else new_store->vmap = nvm;

    prev = nvm;
    vm = vm->next;
  }

  return new_store;
}

/**
 * destroy_varmap(v)
 *
 * free a varmap and recursively destroy those that it points at.
 */
void destroy_varmap(varmap_t *v) {
  if (v == NULL) return;

  destroy_varmap(v->next);
  v->next = NULL;

  destroy_sexp(v->value);

  free(v);
}

/**
 * destroy_store(s)
 *
 * free the memory used by the store.
 */
void destroy_store(slisp_store_t *s) {
  assert(s != NULL);

  destroy_varmap(s->vmap);
  s->vmap = NULL;

  free(s);
}
