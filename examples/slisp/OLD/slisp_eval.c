/*
 * Filename: slisp_eval.c
 * Author  : matt@lanl.gov
 * Created : 19 Mar 2003
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "slisp.h"
#include "slisp_store.h"
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

/*
 * return type from the internal eval
 */
typedef enum {
  SL_PRIMITIVE,
  SL_CLOSURE
} sl_wrap_t;

/*
 * wrapper around primitive values and closures
 */
typedef struct sexp_wrapper {
  sl_wrap_t ty;
  sexp_t *sx;
  slisp_store_t *store;
} sexp_wrap_t;

void dump_sexp_t(sexp_t *s) {
  fprintf(stderr,"\n");
  fprintf(stderr,"s=0x%x\n",(unsigned int)s);
  fprintf(stderr,"s->ty=%d (",s->ty);
  if (s->ty == SEXP_VALUE) fprintf(stderr,"SEXP_VALUE)\n");
  else fprintf(stderr,"SEXP_LIST)\n");
  fprintf(stderr,"s->aty=%d\n",s->aty);
  fprintf(stderr,"s->val=0x%x (",(unsigned int)s->val);
  if (s->val != NULL)
    fprintf(stderr,"%s",s->val);
  fprintf(stderr,")\n");
  fprintf(stderr,"s->next=0x%x\n",(unsigned int)s->next);
  fprintf(stderr,"s->list=0x%x\n",(unsigned int)s->list);
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

#define NUMBUFSIZE 30

/**************************************/
/** macros to keep eval code cleaner **/
/**************************************/

#define CHECK_ARGS(arity,num,_op) if((arity)+1 != (num)) { fprintf(stderr,"OPERATOR %d REQUIRING %d ARGUMENTS RECEIVED %d\n",(_op),(arity),(num)-1); return NULL; }

#define CHECK_NUMERIC_TYPE(ty_arg) if ((ty_arg) == SL_INVALID || (ty_arg) == SL_STRING) { fprintf(stderr,"CANNOT PERFORM OPERATION ON NON-NUMERIC TYPES (%s:%d)\n",__FILE__,__LINE__); return NULL; }

#define SQUOTE_EVAL(sx,tmp) if ((sx)->ty == SEXP_VALUE && (sx)->aty == SEXP_SQUOTE) { (tmp)=parse_sexp((sx)->val,strlen((sx)->val)); destroy_sexp((sx)); (sx)=(tmp); (tmp)=NULL;}

#define CHECK_NONZERO(sx) if (strtod((sx)->val,NULL) == 0.0) { fprintf(stderr,"VALUE MUST BE NON-ZERO\n"); return NULL; }

#define CHECK_NONNEGATIVE(sx) if (strtod((sx)->val,NULL) < 0.0) { fprintf(stderr,"VALUE MUST BE NON-NEGATIVE\n"); return NULL; }

/**************************************/
/**************************************/
/**************************************/


/**
 * Allocate a new sexp_t element representing a list.
 */
sexp_t *new_sexp_list(sexp_t *l) {
  sexp_t *sx = sexp_t_allocate();

  sx->ty = SEXP_LIST;

  sx->list = l;
  sx->next = NULL;

  sx->val = NULL;
  sx->val_used = sx->val_allocated = 0;

  return sx;
}

/**
 * allocate a new sexp_t element representing a value 
 */
sexp_t *new_sexp(char *buf, int bs) {
  sexp_t *sx = sexp_t_allocate();

  sx->ty = SEXP_VALUE;

  sx->val = (char *)malloc(sizeof(char)*(bs+1));
  assert(sx->val != NULL);

  sx->val_used = sx->val_allocated = bs+1;

  strcpy(sx->val,buf);

  sx->list = sx->next = NULL;

#ifdef _DEBUG_
  dump_sexp_t(sx);
#endif /* _DEBUG_ */

  return sx;
}

sexp_t *_slisp_eval(sexp_t *sx, slisp_store_t *store) {
  slisp_op_t op;
  int mult = 1;
  char numbuf[NUMBUFSIZE];
  sexp_t *sx_a, *sx_b, *tmp_sx;
  slisp_val_t ty_a, ty_b;
  int len, d;
#ifdef _DEBUG_
  char debugbuf[BUFSIZ];
#endif /* _DEBUG_ */

  /* NULL returns NULL */
  if (sx == NULL) {
#ifdef _DEBUG_
    fprintf(stderr,"_slisp_eval passed null sx\n");
#endif /* _DEBUG_ */
    return NULL;
  }

#ifdef _DEBUG_
  printf("_slisp_eval: sx=0x%x\n",sx);
  dump_sexp_t(sx);
  print_sexp(debugbuf,BUFSIZ,sx);
  printf("=======>%s\n",debugbuf);
#endif /* _DEBUG_ */

  /* values evaluate to themselves or whatever variable they're bound to */
  if (sx->ty == SEXP_VALUE) {
    if (store->vmap == NULL) 
      return copy_sexp(sx);

    //    d=store->scope_depth;
    
    sx_a = get_variable(sx->val,store,store->scope_depth);

    if (sx_a == NULL) 
      return copy_sexp(sx);
    else {
      d = store->scope_depth - 1;

      sx_b = get_variable(sx_a->val,store,d);
      if (sx_b != NULL) {
        destroy_sexp(sx_a);
        sx_a = sx_b;
        d--;
      }

#ifdef _DEBUG_
      print_sexp(debugbuf,BUFSIZ,sx_a);
      fprintf(stderr,"GOT: %s\n",debugbuf);
#endif /* _DEBUG_ */

      return sx_a;
    }
  }

#ifdef _DEBUG_
  printf("_slisp_eval: pointing at list...\n");
  dump_sexp_t(sx->list);
#endif /* _DEBUG_ */

  if (sx->list->ty == SEXP_LIST) 
    return _slisp_eval(sx->list,store);

  len = sexp_list_length(sx);
  op = tokenize(sx->list);

#ifdef _DEBUG_
  fprintf(stderr,"----> OPERATION %d (%s)\n",op,sx->list->val);
#endif /* _DEBUG_ */

  switch (op) {
  /* LOGICAL OPERATIONS */
  /** unary **/
  case SL_NOT:
    CHECK_ARGS(1,len,op);

    sx_a = _slisp_eval(sx->list->next,store);

    if (sx_a->ty == SEXP_VALUE && strcmp(sx_a->val,"t") == 0)
      sprintf(numbuf,"f");
    else
      sprintf(numbuf,"t");

    destroy_sexp(sx_a);

    return new_sexp(numbuf,strlen(numbuf));
    break;
 
  /** binary **/
  case SL_EQ:
  case SL_LEQ:
  case SL_GEQ:
  case SL_NE:
  case SL_GT:
  case SL_LT:
    CHECK_ARGS(2,len,op);

    sx_a = _slisp_eval(sx->list->next,store);
    sx_b = _slisp_eval(sx->list->next->next,store);
    ty_a = derive_type(sx_a);
    ty_b = derive_type(sx_b);

    if (ty_a == SL_SEXP || ty_b == SL_SEXP) {
      fprintf(stderr,"BOOLEAN TESTS REQUIRE NON-LIST OPERANDS.\n");
      return NULL;
    }

    if (ty_a != ty_b)
      sprintf(numbuf,"f");
    else {

      switch(op) {
      case SL_EQ:
        if (strcmp(sx_a->val,sx_b->val) == 0)
          sprintf(numbuf,"t");
        else
          sprintf(numbuf,"f");
	break;

      case SL_NE:
        if (strcmp(sx_a->val,sx_b->val) != 0)
          sprintf(numbuf,"t");
        else
          sprintf(numbuf,"f");
	break;

      case SL_GEQ:
        if (ty_a == SL_STRING) {
          if (strcmp(sx_a->val,sx_b->val) >= 0)
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
        } else {
          if (strtod(sx_a->val,NULL) >= strtod(sx_b->val,NULL))
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
        }
	break;

      case SL_LEQ:
        if (ty_a == SL_STRING) {
          if (strcmp(sx_a->val,sx_b->val) <= 0)
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
        } else {
          if (strtod(sx_a->val,NULL) <= strtod(sx_b->val,NULL))
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
	}
	break;

      case SL_GT:
        if (ty_a == SL_STRING) {
          if (strcmp(sx_a->val,sx_b->val) > 0)
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
        } else {
          if (strtod(sx_a->val,NULL) > strtod(sx_b->val,NULL))
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
	}
	break;

      case SL_LT:
        if (ty_a == SL_STRING) {
          if (strcmp(sx_a->val,sx_b->val) < 0)
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
        } else {
          if (strtod(sx_a->val,NULL) < strtod(sx_b->val,NULL))
            sprintf(numbuf,"t");
          else
            sprintf(numbuf,"f");
	}
	break;

      default:
	fprintf(stderr,"THIS SHOULD NEVER HAPPEN!\n");
        return NULL;
      }
    }

    destroy_sexp(sx_a);
    destroy_sexp(sx_b);

    return new_sexp(numbuf,strlen(numbuf));
    break;

  /* MINUS and PLUS */
  case SL_MINUS:
    mult = -1;
  case SL_PLUS:
    CHECK_ARGS(2,len,op);

    sx_a = _slisp_eval(sx->list->next,store);
    sx_b = _slisp_eval(sx->list->next->next,store);
    ty_a = derive_type(sx_a);
    ty_b = derive_type(sx_b);
    
    CHECK_NUMERIC_TYPE(ty_a);
    CHECK_NUMERIC_TYPE(ty_b);

    /* float */
    if (ty_a == SL_FLOAT || ty_b == SL_FLOAT)
      sprintf(numbuf,"%f",(strtod(sx_a->val,NULL) + 
                           ((double)mult * strtod(sx_b->val,NULL))));
    /* int */
    else
      sprintf(numbuf,"%d",(atoi(sx_a->val) + (mult *atoi(sx_b->val))));

    destroy_sexp(sx_a);
    destroy_sexp(sx_b);

    return new_sexp(numbuf,strlen(numbuf));
    break;

  /* DIVIDE and MULT */
  case SL_DIVIDE:
  case SL_MULT:
    CHECK_ARGS(2,len,op);

    sx_a = _slisp_eval(sx->list->next,store);
    sx_b = _slisp_eval(sx->list->next->next,store);

    if (sx_a == NULL || sx_b == NULL) dump_store(store);

    assert(sx_a != NULL);
    assert(sx_b != NULL);

    ty_a = derive_type(sx_a);
    ty_b = derive_type(sx_b);

    CHECK_NUMERIC_TYPE(ty_a);
    CHECK_NUMERIC_TYPE(ty_b);

    if (op == SL_DIVIDE) CHECK_NONZERO(sx_b);

    /* division */
    if (op == SL_DIVIDE) {
      sprintf(numbuf,"%f",(strtod(sx_a->val,NULL) / 
                           strtod(sx_b->val,NULL)));
    /* multiplication */
    } else {
      if (ty_a == SL_INT && ty_b == SL_INT)
        sprintf(numbuf,"%d",(atoi(sx_a->val) * atoi(sx_b->val)));
      else
        sprintf(numbuf,"%f",(strtod(sx_a->val,NULL) *
                             strtod(sx_b->val,NULL)));
    }

    destroy_sexp(sx_a);
    destroy_sexp(sx_b);

    return new_sexp(numbuf,strlen(numbuf));
    break;

  case SL_EXP:
    CHECK_ARGS(2,len,op);

    sx_a = _slisp_eval(sx->list->next,store);
    sx_b = _slisp_eval(sx->list->next->next,store);
    ty_a = derive_type(sx_a);
    ty_b = derive_type(sx_b);
    
    CHECK_NUMERIC_TYPE(ty_a);
    CHECK_NUMERIC_TYPE(ty_b);

    if (strtod(sx_b->val,NULL) < 0.0) {
      sprintf(numbuf,"%f",(pow(strtod(sx_a->val,NULL),
                               strtod(sx_b->val,NULL))));
    } else {
      if (ty_a == SL_INT && ty_b == SL_INT)
        sprintf(numbuf,"%d",(int)(pow(strtod(sx_a->val,NULL),
                                      strtod(sx_b->val,NULL))));
      else
        sprintf(numbuf,"%f",(pow(strtod(sx_a->val,NULL),
                                 strtod(sx_b->val,NULL))));
    }

    destroy_sexp(sx_a);
    destroy_sexp(sx_b);

    return new_sexp(numbuf,strlen(numbuf));
    break;

  case SL_SQRT:
    CHECK_ARGS(1,len,op);

    sx_a = _slisp_eval(sx->list->next,store);
    ty_a = derive_type(sx_a);

    CHECK_NUMERIC_TYPE(ty_a);
    CHECK_NONNEGATIVE(sx_a);

    sprintf(numbuf,"%f",(sqrt(strtod(sx_a->val,NULL))));

    destroy_sexp(sx_a);

    return new_sexp(numbuf,strlen(numbuf));
    break;

  case SL_CDR:
  case SL_CAR:
    CHECK_ARGS(1,len,op);

    sx_a = _slisp_eval(sx->list->next,store);

    SQUOTE_EVAL(sx_a,tmp_sx);

    if (sx_a->ty != SEXP_LIST) {
      fprintf(stderr,"CANNOT PERFORM CAR ON NON-LIST EXPRESSION.\n");
      return NULL;
    }

    if (op == SL_CAR) {
      sx_b = copy_sexp(sx_a->list);
      destroy_sexp(sx_a);
      sx_a = sx_b;
      sx_a->next = NULL;
    } else {
      sx_b = new_sexp_list(copy_sexp(sx_a->list->next));
      sx_b->next = NULL;
      destroy_sexp(sx_a);
      sx_a = sx_b;
    }

    return sx_a;
    break;

  case SL_CONS:
    CHECK_ARGS(2,len,op);

    sx_a = _slisp_eval(sx->list->next,store);
    sx_b = _slisp_eval(sx->list->next->next,store);

    SQUOTE_EVAL(sx_a,tmp_sx);
    SQUOTE_EVAL(sx_b,tmp_sx);

    tmp_sx = new_sexp_list(sx_a);
    if (sx_b->ty != SEXP_LIST)
      tmp_sx->list->next = sx_b;
    else
      tmp_sx->list->next = sx_b->list;

    return tmp_sx;
    break;

  case SL_IF:
    CHECK_ARGS(3,len,op);

    sx_a = _slisp_eval(sx->list->next,store);

    if (sx_a->ty != SEXP_VALUE) {
	fprintf(stderr,"IF REQUIRES BOOLEAN TEST\n");
    	return NULL;
    }

    if (strcmp(sx_a->val, "t") == 0) {
        destroy_sexp(sx_a);
        return _slisp_eval(sx->list->next->next,store);
    } else {
        destroy_sexp(sx_a);
        return _slisp_eval(sx->list->next->next->next,store);
    }

    break;

  case SL_LAMBDA:
    if (len < 2) {
      fprintf(stderr,"LAMBDA REQUIRES AT LEAST ONE ARGUMENT.\n");
      return NULL;
    }

    /* no arguments */
    if (len == 2) {
      fprintf(stderr,"LAMBDA W/ NO ARGS\n");
    }

    /* walk to the last expression in the sequence */
    sx_a = sx->list->next; /* arg1 name */
    sx_b = sx->next;       /* arg1 expression */
    while (sx_a->next != NULL) {
#ifdef _DEBUG_
      fprintf(stderr,"VAR:%s.\n",sx_a->val);
#endif /* _DEBUG_ */

      if (sx_b == NULL) {
        fprintf(stderr,"LAMBDA EXPRESSION REQUIRES MORE ARGUMENTS.\n");
        return NULL;
      }

      tmp_sx = sx_b->next;
      sx_b->next = NULL;
      set_variable(sx_a->val,sx_b,store);
      sx_b->next = tmp_sx;

      sx_a = sx_a->next;
      sx_b = sx_b->next;
    }

    while (sx_a->next != NULL) sx_a = sx_a->next;
    tmp_sx = _slisp_eval(sx_a,store);

    sx->next = NULL;
    destroy_sexp(sx);

    return tmp_sx;

    break;   

  case SL_SORT:

  case SL_FOLD:
  case SL_MAP:
    
  default:
    fprintf(stderr,"UNKNOWN OPERATION %d (%s)\n",op,sx->list->val);
    return NULL;
  };
  return NULL;
}

sexp_t *slisp_eval(sexp_t *sx) {
  slisp_store_t *store = NULL;
  sexp_t *ret = NULL;

  store = init_store();

  ret = _slisp_eval(copy_sexp(sx),store);

  destroy_store(store);

  return ret;
}
