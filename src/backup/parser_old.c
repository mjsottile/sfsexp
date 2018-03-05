/**
This software and ancillary information (herein called "SOFTWARE")
called Supermon is made available under the terms described
here.  The SOFTWARE has been approved for release with associated
LA-CC Number LA-CC 99-51.

Unless otherwise indicated, this SOFTWARE has been authored by an
employee or employees of the University of California, operator of the
Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
the U.S. Department of Energy.  The U.S. Government has rights to use,
reproduce, and distribute this SOFTWARE, and to allow others to do so.
The public may copy, distribute, prepare derivative works and publicly
display this SOFTWARE without charge, provided that this Notice and
any statement of authorship are reproduced on all copies.  Neither the
Government nor the University makes any warranty, express or implied,
or assumes any liability or responsibility for the use of this
SOFTWARE.

If SOFTWARE is modified to produce derivative works, such modified
SOFTWARE should be clearly marked, so as not to confuse it with the
version available from LANL.
**/
/** NOTE: This library is part of the supermon project, hence the name
          supermon above. **/
/**
 * Matt's smaller s-expression parsing library
 * Version 0.1.2
 *
 * Written by Matt Sottile (matt@lanl.gov), January 2002.
 ***/
/**
This software and ancillary information (herein called "SOFTWARE")
called Supermon is made available under the terms described
here.  The SOFTWARE has been approved for release with associated
LA-CC Number LA-CC 99-51.

Unless otherwise indicated, this SOFTWARE has been authored by an
employee or employees of the University of California, operator of the
Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
the U.S. Department of Energy.  The U.S. Government has rights to use,
reproduce, and distribute this SOFTWARE, and to allow others to do so.
The public may copy, distribute, prepare derivative works and publicly
display this SOFTWARE without charge, provided that this Notice and
any statement of authorship are reproduced on all copies.  Neither the
Government nor the University makes any warranty, express or implied,
or assumes any liability or responsibility for the use of this
SOFTWARE.

If SOFTWARE is modified to produce derivative works, such modified
SOFTWARE should be clearly marked, so as not to confuse it with the
version available from LANL.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sexpr.h"
#include "faststack.h"

typedef struct parse_stack_data {
  sexpr_t *fst, *lst;
} parse_data_t;

/*
 * create a new continuation
 */
static pcont_t *new_cont() {
  pcont_t *pc;
  pc = (pcont_t *)malloc(sizeof(pcont_t));
  pc->stack = make_stack();
  pc->last_sexpr = NULL;
  pc->val = pc->lastPos = NULL;
  pc->vcur = NULL;
  pc->depth = 0;
  return pc;
}

/*
 * destroy a continuation 
 */
void destroy_cont(pcont_t *pc) {
  free(pc);
}

/*
 * return the last s-expression parsed and stored in a continuation
 */
sexpr_t *last_sexp(pcont_t *pc) {
  sexpr_t *s = NULL;

  if (pc->last_sexpr != NULL) {
    s = pc->last_sexpr;
    pc->last_sexpr = NULL;
  }

  return s;
}

/* 
 * wrapper around cparse_sexp.  assumes s contains a single, complete,
 * null terminated s-expression.  partial sexps or strings containing more
 * than one will act up.
 */
sexpr_t *parse_sexp(char *s, int len) {
  pcont_t *pc = NULL;

  pc = cparse_sexp(s,len,pc);

  return pc->last_sexpr;
}

/**
 * Given a string s with a length len, return a sexpr_t representation
 * of the sexpression in s.  Assumes that s contains a well formed
 * s-expression (IE: for each '(', there exists a ')' somewhere after it
 * in the string).
 */
pcont_t *cparse_sexp(char *s, int len, pcont_t *pc) {
#ifdef _COUNT_MALLOC_
  int dm=0,df=0; /* malloc/free count for data */
  int vm=0,vf=0; /* malloc/free count for val */
#endif
  sexpr_t      *sx_a;
  char         *t = s;
  char         *val, *vcur;
  faststack_t  *stack;
  stack_lvl_t  *lvl;
  parse_data_t *data;
  pcont_t      *cc; /* current continuation */
  register unsigned int depth;

  /***
   *** before we start parsing...
   ***
   *** restore state from continuation!!!
   ***/

  /* make sure current continuation actually exists... */
  if (pc == NULL)
    cc = new_cont(); /* use existing */
  else {
    cc = pc;         /* none passed */
    if (cc->lastPos != NULL) 
      t = cc->lastPos;
  }

  /* get depth count out of continuation */
  depth = cc->depth;

  /* get stack from continuation */
  stack = cc->stack;

  /* if cc->val != NULL, start where we left off */
  if (cc->val != NULL){
    val = cc->val;
    vcur = cc->vcur;
  } else {
    /* allocate a buffer to parsing atoms into */
    val = (char *)malloc(sizeof(char)*MAX_SEXPR_ATOM_SIZE);

    /* set val pointer in continuation */
    cc->val = val;
    cc->vcur = NULL;
  }

  /***
   *** state restored - start parsing like usual
   ***/

#ifdef _COUNT_MALLOC_
  vm++;
#endif

  if (t[0] == ' ' || t[0] == '\t' || t[0] == '\n' || t[0] == '\r') {
    if (cc->vcur != NULL) {
      /* add null terminator */
      vcur[0] = 0;

      /* if it is nonempty... */
      if (&vcur[0] != &val[0]) {
	sx_a = (sexpr_t *)malloc(sizeof(sexpr_t));
#ifdef _COUNT_MALLOC_
	sx_mallocs++;
#endif
	sx_a->ty = VALUE;
	strcpy(sx_a->val,val);
	sx_a->next = NULL;
		
	if (!empty_stack(stack)) {
	  data = (parse_data_t *)top_data(stack); 
	} else {
          fprintf(stderr,"Atom encountered while stack empty.\n");
	}

	/* 
	 * attach new sexpr_t to top stack level
	 */
	if (data->fst == NULL) {
	  data->fst = sx_a;
	  data->lst = sx_a;
	} else {
	  data->lst->next = sx_a;
	  data->lst = sx_a;
	}
      }
    }
    cc->vcur = NULL;
    vcur = NULL;
    /* spin through spaces at the beginning */
    while ((t[0] == ' ' || t[0] == '\t' || t[0] == '\n' || t[0] == '\r') && 
  	   t[0] != 0) t++;
  }

  /* starting condition: t[0] is non-whitespace */
  while (t[0] != 0) {
    /**
     * open paren -- push level onto stack (like recursive call)
     */
    switch (t[0]) {
    default:
      /*
       * otherwise, eat strings.  spin forward through spaces and \n chars,
       * and eat until we see another paren, space, or \n.
       */
      while ((t[0] == ' ' || t[0] == '\t' || t[0] == '\n' || t[0] == '\r') && 
	     t[0] != 0) t++;
      if (cc->vcur != NULL) vcur = cc->vcur;
      else vcur = val;
      while (t[0] != ' ' && t[0] != '\t' && t[0] != '\n' && t[0] != '(' &&
	     t[0] != ')' && t[0] != '\0' && t[0] != '\r') {
	vcur[0] = t[0];

	/*
	 * deal with escape characters.  we accept the following after the \:
	 *
	 *  ),(,",',\,n,t,r,v,f,a,b
	 *
	 * this is a slight extension of the escape sequences stated in
	 * K&R, with ? removed (why escape '?' ?)
	 */
        if (t[0] == '\\') {
	  vcur++; t++;
	  if (t[0] == ')' || t[0] == '(' ||
	      t[0] == '\'' || t[0] == '\"' ||
	      t[0] == '\\' || t[0] == 'n' ||
	      t[0] == 't' || t[0] == 'r' ||
	      t[0] == 'v' || t[0] == 'f' ||
	      t[0] == 'b' || t[0] == 'a') {
	    vcur[0] = t[0];
	    vcur++; t++;
	  } else {
	    fprintf(stderr,"Warning: invalid escape character.\n");
	  }
	} else {
	  vcur++; t++;
	}
	/* end of escape processing */

      }

      /* saw null, val might not be done yet */
      if (t[0] == '\0') {
	cc->vcur = vcur; /* save location in val where we got to. */
	cc->depth = depth; /* save sexpr nesting depth */
	return cc;
      }

      /* add null terminator */
      vcur[0] = 0;

      /* if it is nonempty... */
      if (&vcur[0] != &val[0]) {
	sx_a = (sexpr_t *)malloc(sizeof(sexpr_t));
#ifdef _COUNT_MALLOC_
	sx_mallocs++;
#endif
	sx_a->ty = VALUE;
	strcpy(sx_a->val,val);
	sx_a->next = NULL;
		
	if (!empty_stack(stack)) {
	  data = (parse_data_t *)top_data(stack); 
	} else {
          fprintf(stderr,"Atom encountered while stack empty.\n");
	}

	/* 
	 * attach new sexpr_t to top stack level
	 */
	if (data->fst == NULL) {
	  data->fst = sx_a;
	  data->lst = sx_a;
	} else {
	  data->lst->next = sx_a;
	  data->lst = sx_a;
	}
      }

      cc->vcur = NULL;
      break;

    case '(':
      depth++;
      /* alloc sexpr_t */
      sx_a = (sexpr_t *)malloc(sizeof(sexpr_t));
#ifdef _COUNT_MALLOC_
      sx_mallocs++;
#endif

      /* a paren is the beginning of a list element */
      sx_a->ty = LIST;

      /* list and next empty */
      sx_a->list = NULL;
      sx_a->next = NULL;

      /* attach new list to current stack top before moving on. */
      if (stack->height < 1) {
	data = (parse_data_t *)malloc(sizeof(parse_data_t));
#ifdef _COUNT_MALLOC_
	dm++;
#endif
	data->fst = sx_a; data->lst = sx_a;
	push(stack,data);
      } else {
	data = (parse_data_t *)top_data(stack); 
	if (data->lst != NULL) 
	  data->lst->next = sx_a;
	else
	  data->fst = sx_a;
	data->lst = sx_a;
      }

      /* now start the next level with an empty sequence of sexprs */
      data = (parse_data_t *)malloc(sizeof(parse_data_t));
#ifdef _COUNT_MALLOC_
      dm++;
#endif

      data->fst = NULL; data->lst = NULL;

      /* push a new level on the stack */
      push(stack, data);
	  
      /* next char... */
      t++;
      break;
    case ')':
      cc->vcur = NULL;
      depth--;
      /* pop the stack */
      lvl = pop(stack);
      data = (parse_data_t *)lvl->data;
      sx_a = data->fst;
      free(data);
#ifdef _COUNT_MALLOC_
      df++;
#endif
        
      /* if stack not null, free the level we were in.  then take the
	 sequence of sexpr_t structs for that level, and attach them
	 to the last element of THIS level as the list for that list
	 element. */
      if (stack->top != NULL) {
	data = (parse_data_t *)top_data(stack); 
        data->lst->list = sx_a;
      } else {
	break;
      }
	  
      t++;
      break;
    }

    if (depth == 0) break;
  }

  cc->lastPos = t;
  if (depth > 0) {
    cc->vcur = vcur; /* save location in val where we got to. */
    cc->depth = depth; /* save sexpr nesting depth */
    cc->lastPos = NULL;
    return cc;
  }

  /**
   * pop everything else off of the stack.  Usually there is only one thing
   * left when we get out of the loop above, so this only iterates once.
   */  
  while (stack->top != NULL) {
    lvl = pop(stack);
    data = (parse_data_t *)lvl->data;
    sx_a = data->fst;
    free(data);
#ifdef _COUNT_MALLOC_
    df++;
#endif
  }

  /* 
     if we're not quite done with the string yet, save a pointer to where
     we got to.
  */
  if (t[0] == 0) {
    cc->lastPos = NULL;
  }

  /* remove pointer to val buffer */
  cc->val = NULL;

  /* save the sexpr we parsed. */
  cc->last_sexpr = sx_a;

  /* free the val buffer */
  free(val);

  /* memory tracking stuff */
#ifdef _COUNT_MALLOC_
  vf++;
#endif

#ifdef _COUNT_MALLOC_
  fprintf(stderr,"DM:%d // DF:%d\n",dm,df);
  fprintf(stderr,"VM:%d // VF:%d\n",vm,vf);
#endif

  /* free the stack */
  destroy_stack(stack);

  /* return the s-expression */
  return cc;
}

