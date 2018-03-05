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

#include "slisp.h"
#include "slisp_util.h"
#include "slisp_env.h"
#include "slisp_memman.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

sexp_t *_slisp_eval(sexp_t *s, slisp_env_t *e);

sexp_t *slisp_eval(sexp_t *sx) {
  /*  return _slisp_eval(deep_copy_sexp(sx),NULL); */
  return _slisp_eval(sx,NULL);
}

sexp_t *new_sexp_bool(int i) {
  sexp_t *sx;
  sx = (sexp_t *)malloc(sizeof(sexp_t));
  assert(sx != NULL);
  MEM_CHECKPOINT(sx);

  sx->ty = SEXP_VALUE;
  sx->aty = SEXP_BASIC;
  sx->val_allocated = 2;
  sx->val_used = 2;
  sx->val = (char *)malloc(sizeof(char)*2);
  assert(sx->val != NULL);

  if (i == 1) sx->val[0] = 't';
  else sx->val[0] = 'f';
  sx->val[1] = '\0';
  
  return sx;
}

#define SX_TRUE  new_sexp_bool(1)
#define SX_FALSE new_sexp_bool(0)
#define SX_NIL   new_sexp_atom("nil",4,SEXP_BASIC)

#ifdef _ENABLE_WARNINGS_
#define WARNING(str) fprintf(stderr,"WARNING::%s\n",(str));
#else
#define WARNING(str) { }
#endif

#define DEBUG(str) fprintf(stderr,"[%s:%d]::%s\n",__FILE__,__LINE__,(str));

/********
 **
 ** don't look at the macros below.  they may cause illness, or an urge
 ** to wring my neck.  neither are recommended outcomes.
 **
 **/

/** typeless comparison operator.  prevent duplication of code for SL_???
    operators for each supported type. **/
#define COMPARE_OPERATOR(a,b,op) { switch((op)) { case SL_EQ: if ((a) == (b)) return SX_TRUE; else return SX_FALSE; break; case SL_NE: if ((a) != (b)) return SX_TRUE; else return SX_FALSE; case SL_GT: if ((a) > (b)) return SX_TRUE; else return SX_FALSE; case SL_LT: if ((a) < (b)) return SX_TRUE; else return SX_FALSE; case SL_GEQ: if ((a) >= (b)) return SX_TRUE; else return SX_FALSE; case SL_LEQ: if ((a) <= (b)) return SX_TRUE; else return SX_FALSE; default: break;} }

/** typeless arithmetic operator **/
#define ARITH_OPERATOR(a,b,c,op) { switch((op)) { case SL_PLUS: (c)=(a)+(b); break; case SL_MINUS: (c)=(a)-(b); break; case SL_MULT: (c)=(a)*(b); break; case SL_DIVIDE: (c)=(a)/(b); break; case SL_EXP: (c)=pow((double)(a),(double)(b)); break; default: break; } }

/**
 ** Ok - you can look again.
 **
 ********/

/** evaluate if operations **/
sexp_t *eval_conditional(sexp_t *sx, slisp_env_t *env) {
  sexp_t *tresult, *test, *tclause, *eclause;

  test = sx->list->next;
  if (test == NULL) {
    DEBUG("bad if arguments.");
    return NULL;
  }
  tclause = test->next;
  if (tclause == NULL) {
    DEBUG("bad then clause in if");
    return NULL;
  }
  eclause = tclause->next;
  /* this is optional.  if it is null, return NULL. */

  tresult = _slisp_eval(test,env);

  if (tresult == NULL) {
    DEBUG("conditional test evaluation error.");
    return NULL;
  }

  if (tresult->aty != SEXP_VALUE) {
    DEBUG("conditional test evaluated to non-primitive type.");
    return NULL;
  }

  if (tresult->val_allocated > 1 && tresult->val[1] == '\0') {
    if (tresult->val[0] == 't') {
      destroy_sexp(tresult);
      FREE_CHECKPOINT(tresult);
      return _slisp_eval(tclause,env);
    } else if (tresult->val[0] == 'f') {
      destroy_sexp(tresult);
      FREE_CHECKPOINT(tresult);
      if (eclause == NULL) return SX_NIL;
      else return _slisp_eval(eclause,env);
    } else {
      DEBUG("conditional test evaluated to non-boolean value.");
      return NULL;
    }
  }

  DEBUG("unknown error in conditional evaluation.");
  return NULL;
}

/** evaluate binary operators **/
sexp_t *eval_binop(sexp_t *sx, slisp_op_t op, slisp_env_t *env) {
  sexp_t *operator, *a1, *a2;
  slisp_val_t t1, t2;
  int i1, i2;
  double f1, f2;
  int reeval = 0x00;

  operator = sx->list;  
  a1 = operator->next;
  if (a1 == NULL) {
    DEBUG("first argument of binary operator undefined.\n");
    return NULL;
  }

  a2 = a1->next;
  if (a2 == NULL) {
    DEBUG("second argument of binary operator undefined.\n");
    return NULL;
  }

  t1 = derive_type(a1);
  t2 = derive_type(a2);

  if (t1 == SL_SEXP) {
    a1 = _slisp_eval(a1,env);
    reeval |= 0x01;
    t1 = derive_type(a1);
  }

  if (t2 == SL_SEXP) {
    a2 = _slisp_eval(a2,env);
    reeval |= 0x02;
    t2 = derive_type(a2);
  }
  
  if (t1 != t2) {
    WARNING("comparison between incompatible types always false.");
    return SX_FALSE;
  } else {
    switch (t1) {
    case SL_INT:
      i1 = atoi(a1->val);
      i2 = atoi(a2->val);
      
      if (reeval != 0) {
        if ((reeval & 0x01) == 0x01) {
          destroy_sexp(a1);
          FREE_CHECKPOINT(a1);
        }
        if ((reeval & 0x02) == 0x02) {
          destroy_sexp(a2);
          FREE_CHECKPOINT(a2);
        }
      }

      COMPARE_OPERATOR(i1,i2,op);

      break;

    case SL_FLOAT:
      f1 = strtod(a1->val,NULL);
      f2 = strtod(a2->val,NULL);

      if (reeval != 0) {
        if ((reeval & 0x01) == 0x01) {
          destroy_sexp(a1);
          FREE_CHECKPOINT(a1);
        }
        if ((reeval & 0x02) == 0x02) {
          destroy_sexp(a2);
          FREE_CHECKPOINT(a2);
        }
      }

      COMPARE_OPERATOR(f1,f2,op);

      break;

    case SL_STRING:
      i1 = strcmp(a1->val,a2->val);
      i2 = 0;
      
      if (reeval != 0) {
        if ((reeval & 0x01) == 0x01) {
          destroy_sexp(a1);
          FREE_CHECKPOINT(a1);
        }
        if ((reeval & 0x02) == 0x02) {
          destroy_sexp(a2);
          FREE_CHECKPOINT(a2);
        }
      }

      COMPARE_OPERATOR(i1,i2,op);

      break;

    case SL_SEXP:
      WARNING("comparison between sexp elements always false.");
      return SX_FALSE;

      break;
    default:
      DEBUG("never should be here!!!");
    }
  }

  return SX_FALSE;
}

/** evaluate binary arithmetic **/
sexp_t *eval_binarith(sexp_t *sx, slisp_op_t op, slisp_env_t *env) {
  sexp_t *operator, *a1, *a2, *retval;
  slisp_val_t t1, t2;
  int i1, i2, ires;
  double f1, f2, fres;
  char cbuf[30];
  int reeval = 0;

  operator = sx->list;  
  a1 = operator->next;
  if (a1 == NULL) {
    DEBUG("first argument of binary operator undefined.\n");
    return NULL;
  }

  a2 = a1->next;
  if (a2 == NULL) {
    DEBUG("second argument of binary operator undefined.\n");
    return NULL;
  }

  t1 = derive_type(a1);
  t2 = derive_type(a2);

  if (t1 == SL_SEXP) {
    a1 = _slisp_eval(a1,env);
    t1 = derive_type(a1);
    reeval |= 0x01;
  }

  if (t2 == SL_SEXP) {
    a2 = _slisp_eval(a2,env);
    t2 = derive_type(a2);
    reeval |= 0x02;
  }
  
  if (t1 == SL_STRING || t2 == SL_STRING) {
    DEBUG("cannot perform arithmetic on strings."); /* idiot */
    return NULL;
  }

  if (t1 == SL_SEXP || t1 == SL_INVALID || 
      t2 == SL_SEXP || t2 == SL_INVALID) {
    DEBUG("invalid arguments for arithmetic operator.");
    return NULL;
  }
  
  if (t1 == t2 && t1 == SL_INT) {
    i1 = atoi(a1->val);
    i2 = atoi(a2->val);
    
    if (reeval != 0) {
      if ((reeval & 0x01) == 0x01) {
        destroy_sexp(a1);
        FREE_CHECKPOINT(a1);
      }
      if ((reeval & 0x02) == 0x02) {
        destroy_sexp(a2);
        FREE_CHECKPOINT(a2);
      }
    }

    ARITH_OPERATOR(i1,i2,ires,op);
    sprintf(cbuf,"%d",ires);
    retval = new_sexp_atom(cbuf,strlen(cbuf),SEXP_BASIC);
    MEM_CHECKPOINT(retval);

    return retval;
  } else {
    f1 = strtod(a1->val,NULL);
    f2 = strtod(a2->val,NULL);

    if (reeval != 0) {
      if ((reeval & 0x01) == 0x01) {
        destroy_sexp(a1);
        FREE_CHECKPOINT(a1);
      }
      if ((reeval & 0x02) == 0x02) {
        destroy_sexp(a2);
        FREE_CHECKPOINT(a2);
      }
    }

    ARITH_OPERATOR(f1,f2,fres,op);
    sprintf(cbuf,"%f",fres);
    retval = new_sexp_atom(cbuf,strlen(cbuf), SEXP_BASIC);
    MEM_CHECKPOINT(retval);
    return retval;
  }

  DEBUG("unknown error evaluating arithmetic operator.");
  return NULL;
}

sexp_t *eval_listop(sexp_t *sx, slisp_op_t op, slisp_env_t *env) {
  sexp_t *retval, *l1, *l2;

  if (op == SL_CAR || op == SL_CDR) {
    l1 = sx->list->next;
    if (l1 == NULL) {
      DEBUG("invalid argument for car/cdr");
      return NULL;
    }

    l1 = _slisp_eval(l1,env);
    if (l1->ty == SEXP_LIST || 
        (l1->ty == SEXP_VALUE && l1->aty == SEXP_SQUOTE)) {
      if (l1->ty == SEXP_VALUE && l1->aty == SEXP_SQUOTE) {
        l1 = parse_sexp(l1->val,l1->val_used);
        if (l1->ty != SEXP_LIST) {
          DEBUG("car/cdr squote eval'd to non-list.");
          return NULL;
        }
      }
      if (op == SL_CAR) {
        l2 = l1->list->next;
        l1->list->next = NULL;
        retval = deep_copy_sexp(l1->list);
        l1->list->next = l2;
        MEM_CHECKPOINT(retval);
        return retval;
      } else {
        assert(l1->list != NULL);
        retval = deep_copy_sexp(l1->list->next);
        MEM_CHECKPOINT(retval);
        return retval;
      }
    } else {
      DEBUG("car/cdr argument must evaluate to a list.");
      return NULL;
    }
  } else {
  }

  DEBUG("unknown error in eval_listop");
  return NULL;
}

sexp_t *_slisp_eval(sexp_t *sx, slisp_env_t *env) {
  sexp_t *retval = NULL;
  slisp_op_t op;
  sexp_t *tmpsx;
  double d; /* ??? */
  char cbuf[30];
  int reeval = 0;

  assert(sx != NULL);

  /* check type of element.  if it is an atom, return it.  otherwise, treat
     lists as (function arg0 arg1 ... argn) */
  if (sx->ty == SEXP_VALUE) {
    retval = deep_copy_sexp(sx);
    MEM_CHECKPOINT(retval);
    return retval;
  }

  /* tokenize head of list */
  op = tokenize(sx->list);
  switch (op) {
  case SL_EQ:
  case SL_GT:
  case SL_LT:
  case SL_NE:
  case SL_GEQ:
  case SL_LEQ:
    /** binary operator **/
    return eval_binop(sx,op,env);
    break;

  case SL_NOT:
    /** unary operator **/
    tmpsx = sx->list->next;

    if (tmpsx == NULL) {
      DEBUG("argument error for unary not operator.");
      return NULL;
    }
    if (tmpsx->ty == SEXP_LIST) {
      reeval = 1;
      tmpsx = _slisp_eval(tmpsx,env);
      if (tmpsx->ty == SEXP_LIST) {
        DEBUG("cannot evaluate not operator on list argument.");
        return NULL;
      }
    }
    if (tmpsx->val_allocated > 1 && tmpsx->val[1] == '\0') {
      if (tmpsx->val[0] == 't') {
        if (reeval == 1) {
          destroy_sexp(tmpsx);
          FREE_CHECKPOINT(tmpsx);
        }
        return SX_FALSE;
      } else {
        if (tmpsx->val[0] == 'f') {
          if (reeval == 1) {
            destroy_sexp(tmpsx);
            FREE_CHECKPOINT(tmpsx);
          }
          return SX_TRUE;
        } else {
          DEBUG("invalid argument for not operator.");
          if (reeval == 1) {
            destroy_sexp(tmpsx);
            FREE_CHECKPOINT(tmpsx);
          }
          return NULL;
        }
      }
    }

    DEBUG("error evaluating not operator.");
    return NULL;

    break;

  case SL_PLUS:
  case SL_MINUS:
  case SL_MULT:
  case SL_DIVIDE:
  case SL_EXP:
    /** binary arithmetic **/
    return eval_binarith(sx,op,env);
    break;

  case SL_SQRT:
    /** unary arithmetic **/
    tmpsx = sx->list->next;
    if (tmpsx == NULL) {
      DEBUG("missing argument for sqrt.");
      return NULL;
    }

    switch (derive_type(tmpsx)) {
    case SL_INT:
    case SL_FLOAT:
      d = sqrt(strtod(tmpsx->val,NULL));
      sprintf(cbuf,"%f",d);
      tmpsx = new_sexp_atom(cbuf,30, SEXP_BASIC);
      return tmpsx;
    default:
      DEBUG("bad type in sqrt");
      return NULL;
    }

    DEBUG("unknown error in eval for sqrt.");
    return NULL;
    break;

  case SL_CONS:
  case SL_CDR:
  case SL_CAR:
    /** list stuff **/
    return eval_listop(sx,op,env);
    break;

  case SL_FOLD:
  case SL_MAP:
    /** function application over lists **/
    break;

  case SL_SORT:
    break;

  case SL_IF:
    return eval_conditional(sx,env);
    break;

  case SL_LAMBDA:
    break;

  default:
    fprintf(stderr,"EVAL: unknown token\n");
  }

  return retval;
}
