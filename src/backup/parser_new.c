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
 * Version 0.2.0
 *
 * Written by Matt Sottile (matt@lanl.gov), January 2002.
 ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sexpr.h"
#include "faststack.h"

/**
 * this structure is pushed onto the stack so we can keep track of the
 * first and last elements in a list.
 */
typedef struct parse_stack_data
{
  sexpr_t *fst, *lst;
}
parse_data_t;

/*
 * create a new continuation
 */
static pcont_t *
new_cont ()
{
  pcont_t *pc;
  pc = (pcont_t *) malloc (sizeof (pcont_t));
  pc->stack = make_stack ();
  pc->last_sexpr = NULL;
  pc->val = pc->lastPos = NULL;
  pc->vcur = NULL;
  pc->depth = 0;
  return pc;
}

/*
 * destroy a continuation 
 */
void
destroy_cont (pcont_t * pc)
{
  free (pc);
}

/*
 * return the last s-expression parsed and stored in a continuation
 */
sexpr_t *
last_sexp (pcont_t * pc)
{
  sexpr_t *s = NULL;

  if (pc->last_sexpr != NULL)
    {
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
sexpr_t *
parse_sexp (char *s, int len)
{
  pcont_t *pc = NULL;

  pc = cparse_sexp (s, len, pc);

  return pc->last_sexpr;
}

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

void addatom (faststack_t * stack, char *val, int aty);
sexpr_t *newatom (char *val);
sexpr_t *newlist ();

pcont_t *
cparse_sexp (char *str, int len, pcont_t *lc)
{
  char *t, *s;
  register unsigned int state = 1;
  register unsigned int depth = 0;
  register unsigned int qdepth = 0;
  pcont_t *cc;
  char *val, *vcur;
  sexpr_t *sx;
  faststack_t *stack;
  parse_data_t *data;
  stack_lvl_t *lvl;

  /* make sure non-null string */
  if (str == NULL)
    return;

  /* first, if we have a non null continuation passed in, restore state. */
  if (lc != NULL) {
	  cc = lc;
	  val = cc->val;
	  vcur = cc->vcur;
	  state = cc->state;
	  depth = cc->depth;
	  qdepth = cc->qdepth;
	  stack = cc->stack;
	  s = str;
	  if (cc->lastPos != NULL)
	    t = cc->lastPos;
	  else
		t = s;
  } else {
	  /* new continuation... */
	  cc = (pcont_t *)malloc(sizeof(pcont_t));

	  /* allocate atom buffer */
	  cc->val = val = (char *)malloc(sizeof(char)*MAX_SEXPR_ATOM_SIZE);
	  vcur = val;

	  /* allocate stack */
	  cc->stack = stack = make_stack();

	  /* t is temp pointer into s for current position */
	  s = str;
	  t = s;
  }

  /*==================*/
  /* main parser loop */
  /*==================*/
  while (t[0] != '\0')
    {

    /* based on the current state in the FSM, do something */
    switch (state)
	{
	case 1:
	  switch (t[0])
	    {
	      /* space,tab,CR,LF considered white space */
	    case ' ':
	    case '\t':
	    case '\n':
	    case '\r':
	      t++;
	      break;
	      /* enter state 2 for open paren */
	    case '(':
	      state = 2;
	      t++;
	      break;
	      /* enter state 3 for close paran */
	    case ')':
	      state = 3;
	      break;
	      /* begin quoted string - enter state 5 */
	    case '\"':
	      state = 5;
	      /* set cur pointer to beginning of val buffer */
	      vcur = val;
	      t++;
	      break;
	      /* single quote - enter state 7 */
	    case '\'':
	      state = 7;
	      t++;
	      break;
	      /* other characters are assumed to be atom parts */
	    default:
	      /* set cur pointer to beginning of val buffer */
	      vcur = val;
	      /* enter state 4 */
	      state = 4;
	      break;
	    }
	  break;
	case 2:
	  /* open paren */
	  depth++;
	  sx = newlist ();
	  if (stack->height < 1)
	    {
	      data = (parse_data_t *) malloc (sizeof (parse_data_t));
	      data->fst = data->lst = sx;
	      push (stack, data);
	    }
	  else
	    {
	      data = (parse_data_t *) top_data (stack);
	      if (data->lst != NULL)
		data->lst->next = sx;
	      else
		data->fst = sx;
	      data->lst = sx;
	    }

	  data = (parse_data_t *) malloc (sizeof (parse_data_t));
	  data->fst = data->lst = NULL;
	  push (stack, data);

	  state = 1;
	  break;
	case 3:
	  /** close paren **/
	  t++;
	  depth--;

	  lvl = pop (stack);
	  data = (parse_data_t *) lvl->data;
	  sx = data->fst;
	  free (data);

	  if (stack->top != NULL)
	    {
	      data = (parse_data_t *) top_data (stack);
	      data->lst->list = sx;
	    }
	  else
	    {
	      fprintf (stderr, "Hmmm. Stack->top is null.\n");
	    }

	  state = 1;

	  /** if depth = 0 then we finished a sexpr, and we return **/
	  if (depth == 0) {
		  cc->val = val;
		  cc->vcur = vcur;
		  cc->lastPos = t;
		  cc->depth = depth;
		  cc->qdepth = qdepth;
		  cc->state = state;
		  cc->stack = stack;
          while (stack->top != NULL)
          {
		      lvl = pop (stack);
              data = (parse_data_t *) lvl->data;
              sx = data->fst;
              free (data);
          }
		  cc->last_sexpr = sx;
		  return cc;
	  }
	  break;
	case 4:
	  if (!((t[0] >= 'a' && t[0] <= 'z') ||
		(t[0] >= 'A' && t[0] <= 'Z') ||
		(t[0] >= '0' && t[0] <= '9') || (t[0] == '!')))
	    {
	      vcur[0] = '\0';
	      addatom (stack, val, BASIC);
	      state = 1;
	    }
	  else
	    {
	      vcur[0] = t[0];
	      vcur++;
	      t++;
	    }
	  break;
	case 5:
	  if (t[0] == '\"')
	    {
	      state = 6;
	      vcur[0] = '\0';
	      addatom (stack, val, DQUOTE);
	    }
	  else
	    {
	      vcur[0] = t[0];
	      vcur++;
	    }
	  t++;
	  break;
	case 6:
	  vcur = val;
	  state = 1;
	  break;
	case 7:
	  if (t[0] == '\"')
	    {
	      state = 5;
	      vcur = val;
	    }
	  else if (t[0] == '(')
	    {
	      vcur = val;
	      state = 8;
	    }
	  else
	    {
	      vcur = val;
	      state = 4;
	    }
	  break;
	case 8:
	  if (t[0] == '(')
	    {
	      qdepth++;
	    }
	  else if (t[0] == ')')
	    {
	      qdepth--;
	      state = 9;
	    }
	  vcur[0] = t[0];
	  vcur++;
	  t++;
	  break;
	case 9:
	  if (qdepth == 0)
	    {
	      state = 1;
	      vcur[0] = '\0';
	      addatom (stack, val, SQUOTE);
	    }
	  else
	    state = 8;
	  break;
	default:
	  fprintf (stderr, "Unknown state %d\n", state);
	  break;
	}
  }

  if (depth == 0) {
    cc->val = val;
    cc->vcur = vcur;
    cc->lastPos = t;
	cc->depth = depth;
	cc->qdepth = qdepth;
	cc->state = state;
	cc->stack = stack;
    while (stack->top != NULL)
    {
      lvl = pop (stack);
      data = (parse_data_t *) lvl->data;
      sx = data->fst;
      free (data);
    }
    cc->last_sexpr = sx;
  } else {
    cc->val = val;
    cc->vcur = vcur;
	if (t[0] == '\0')
		cc->lastPos = NULL;
	else
		cc->lastPos = t;
	cc->depth = depth;
	cc->qdepth = qdepth;
	cc->state = state;
	cc->stack = stack;
  }

  return cc;
}

void
addatom (faststack_t * stack, char *val, int aty)
{
  parse_data_t *data;
  sexpr_t *sx_a = (sexpr_t *) newatom (val);
  sx_a->aty = aty;

  if (!empty_stack (stack))
    {
      data = (parse_data_t *) top_data (stack);
      if (data->fst == NULL)
	{
	  data->fst = data->lst = sx_a;
	}
      else
	{
	  data->lst->next = sx_a;
	  data->lst = sx_a;
	}
    }
  else
    {
      fprintf (stderr, "Atom encountered while stack empty.\n");
    }
}

sexpr_t *
newatom (char *val)
{
  sexpr_t *sx_a = (sexpr_t *) malloc (sizeof (sexpr_t));
  sx_a->ty = VALUE;
  strcpy (sx_a->val, val);
  sx_a->next = NULL;
  return sx_a;
}

sexpr_t *
newlist ()
{
  sexpr_t *sx_a = (sexpr_t *) malloc (sizeof (sexpr_t));
  sx_a->ty = LIST;
  sx_a->next = NULL;
  sx_a->list = NULL;
  return sx_a;
}
