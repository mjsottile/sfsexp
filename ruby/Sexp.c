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


#include "ruby.h"
#include "sexp.h"

/**
 ** below are rudimentary Ruby bindings for the sexp library
 **
 ** Matt Sottile (matt@lanl.gov) / 5.26.2003
 **/

VALUE cSexp;

/*
 * enumeration and inferrence routine used to guess what the strings
 * represent in detail regarding string vs int vs float.
 */ 
typedef enum { SVAL_STRING, SVAL_INTEGER, SVAL_REAL, SVAL_NONE } sval_type;

/* inferrence routine - about the umpteenth version of this code.  never
 * can remember where it was written the LAST time I wrote it...
 */
static sval_type infer_sval_type(sexp_t *sx) {
  char *c;
  int ishex = 0;
  int sawdecimal = 0;
  int isnegative = 0;

  /* null sx, sx->val, or sx being a list means no type */
  if (sx == NULL || sx->val == NULL || sx->ty != SEXP_VALUE)
    return SVAL_NONE;

  /* beginning of val string */
  c = sx->val;

  /* start with -?  Might be a negative number. */
  if (c[0] == '-') {
      isnegative = 1;
      c++;
  }

  /* start with 0?  Might be hex. */
  if (c[0] == '0') {
    c++;
    /* follow 0 with x or X?  Better chance that it is hex */
    if (c[0] == 'x' || c[0] == 'X') {
      /* hex numbers don't start with a minus sign! */
      if (isnegative == 1) {
	  return SVAL_STRING;
      }
      c++;
      if ((c[0] >= '0' && c[0] <= '9') || 
          ((c[0] >= 'a' && c[0] <= 'f') || 
           (c[0] >= 'A' && c[0] <= 'F'))) 
        /* starts with 0x and a hex digit.  so far, it looks like a hex
           number */
        ishex = 2;
      else
        /* string starting with 0x and a non-hex caracter -- must be
         * a string.
         */
        return SVAL_STRING; 
    } else 
      ishex = 0;
  }

  /* loop over each character.  so far, we know if sx->val starts like a
   * hex number or not, and if not, whether or not it might be a negative
   * number.
   */
  while (c[0] != '\0') {
    if (ishex == 1) {
      if (!((c[0] >= '0' && c[0] <= '9') ||
            ((c[0] >= 'a' && c[0] <= 'f') || 
             (c[0] >= 'A' && c[0] <= 'F'))))
        return SVAL_STRING;
    } else {
      /* not hex */
      if (c[0] == '.') {
        if (sawdecimal == 0) sawdecimal = 1;
        else return SVAL_STRING; /* 2 '.'s mean non-numeric */
      } else {
        if (!(c[0] >= '0' && c[0] <= '9')) {
          return SVAL_STRING; /* not a decimal digit, and not hex, so... */
        }
      }
    }
    c++;
  }

  if (ishex == 1) return SVAL_INTEGER;

  if (sawdecimal == 1) return SVAL_REAL;

  return SVAL_INTEGER;
}

/* destructor called by ruby when cleaning up to deal with the sexp_t
 * stashed away in the object 
 */
static void sexp_rubyfree(void *s) {
  sexp_t *sx = (sexp_t *)s;
  destroy_sexp(sx);
}

/* given a sexp_t, recursively turn it into a ruby array of strings.
 * This is not sufficient to deal with DQUOTE and SQUOTE atoms.
 * This needs to be fixed eventually.  Likely by storing either:
 *   1. A second array corresponding to the first of sexp_t types
 *   2. Replacing string elements in the array(s) with a record type
 *      containing the string and the type.
 */
static VALUE sexp_to_array(sexp_t *sx, int aggressive_typing) {
  VALUE     a = rb_ary_new(); /* create array */
  sexp_t   *s = sx;
  sval_type svt;
  int       i;
  double    d;

  while (s != NULL) {
    if (s->ty == SEXP_LIST) {
      rb_ary_push(a, sexp_to_array(s->list, aggressive_typing));
    } else {
      if (aggressive_typing == 1) {
        svt = infer_sval_type(s);
        switch (svt) {
        case SVAL_INTEGER:
          i = atoi(s->val);
          rb_ary_push(a, INT2FIX(i));
          break;
        case SVAL_REAL:
          d = strtod(s->val,NULL);
          rb_ary_push(a, rb_float_new(d));
          break;
        case SVAL_NONE:
          rb_fatal("ERROR: infer_sval_type => SVAL_NONE for array elt.\n");
          break;
        default:
          rb_ary_push(a, rb_str_new2(s->val));
        }
      } else { /* no aggressive typing - everything is a string */
        rb_ary_push(a, rb_str_new2(s->val));
      }
    }
    s = s->next;
  }
  
  return a;
}

/* given a string, parse it and create the corresponding ruby object
 * for the sexp_t that results.
 */
VALUE sexp_new(VALUE class, VALUE str) {
  sexp_t *sx;
  char *ptr;
  int len;
  VALUE argv[2];
  VALUE td;

  /* make sure it is a string */
  Check_Type(str, T_STRING);

  /* grab the length and base pointer to the string */
  ptr = rb_str2cstr(str, (long *)&len);  
  
  /* parse the string */
  if (len == 0) {
    sx = NULL;
  } else {
    sx = parse_sexp(ptr,len);
  }

  /* stash the sexp_t away in the ruby object */
  td = Data_Wrap_Struct(class, 0, sexp_rubyfree, sx);

  /* set arguments to init up - argv[0] is the original string,
     argv[1] is the array representing it in ruby space. */
  argv[0] = str;

  /* turn the sexp_t into an array */
  if (sx == NULL) {
    argv[1] = rb_ary_new(); /* empty */
  } else {
    if (sx->ty == SEXP_LIST)
      argv[1] = sexp_to_array(sx->list,1);
    else
      argv[1] = sexp_to_array(sx,1);
  }

  /* call the ruby initialize method */
  rb_obj_call_init(td, 2, argv);

  /* return the instance of the ruby object */
  return td;
}

/* initialize expects the original string and the array created from
 * the parsed sexp_t structure.
 */
static VALUE sexp_init(VALUE self, VALUE str, VALUE ary) {
  rb_iv_set(self, "@str", str);
  rb_iv_set(self, "@ary", ary);
  return self;
}

/* given an array representing a s-expression, recursively walk it and
 * string together an equivalent sexp_t representation.  This routine
 * suffers from the same issues mentioned above related to atom
 * type details.
 */
static sexp_t *sexp_unparse_array(VALUE val) {
  sexp_t *sx, *s;
  VALUE v;
  char *b;
  int bs;
  char buf[32];

  /* initialize s to be safe */
  s = NULL;

  /* makes no sense to pass an atom in here... */
  Check_Type(val, T_ARRAY);
  
  /* create a new list with nothing in it.  We know that this is an
   * array being passed in, so we must start with a list.
   */
  sx = new_sexp_list(NULL);

  /* pop elements off from the front of the array one at a time */
  v = rb_ary_shift(val);
  while (!NIL_P(v)) {
    switch (TYPE(v)) {
      /* array? make recursive call */
    case T_ARRAY:
      if (sx->list == NULL) {
        s = sexp_unparse_array(v);
        sx->list = s;
      } else {
        s->next = sexp_unparse_array(v);
        s = s->next;
      }
      break;

      /* int */
    case T_FIXNUM:
      sprintf(buf,"%ld",FIX2LONG(v));
      b = buf;
      bs = strlen(buf);

      if (sx->list == NULL) {
        s = new_sexp_atom(b,bs);
        sx->list = s;
      } else {
        s->next = new_sexp_atom(b,bs);
        s = s->next;
      }
      
      break;

      /* int */
    case T_BIGNUM:
      sprintf(buf,"%ld",NUM2LONG(v));
      b = buf;
      bs = strlen(buf);

      if (sx->list == NULL) {
        s = new_sexp_atom(b,bs);
        sx->list = s;
      } else {
        s->next = new_sexp_atom(b,bs);
        s = s->next;
      }
      
      break;

      /* floating point */
    case T_FLOAT:
      /* ick - there is a better way to get b and bs */
      sprintf(buf,"%f",NUM2DBL(v));
      b = buf;
      bs = strlen(buf);

      if (sx->list == NULL) {
        s = new_sexp_atom(b,bs);
        sx->list = s;
      } else {
        s->next = new_sexp_atom(b,bs);
        s = s->next;
      }

      break;

      /* string */
    case T_STRING:
      b = rb_str2cstr(v,(long *)&bs);

      if (sx->list == NULL) {
        s = new_sexp_atom(b,bs);
        sx->list = s;
      } else {
        s->next = new_sexp_atom(b,bs);
        s = s->next;
      }

      break;
    default:
      /* error? */
      /* who cares - for now, fatal error - GCC doesn't like it if this 
         bit of the switch is empty... */
      rb_fatal("Very bad contents of array!\n");
    }

    /* pop the next */
    v = rb_ary_shift(val);
  }

  /* return sx */
  return sx;
}

/* unparse the ary representation of the s-expression into a sexp_t and
 * then into a char *.  In the process, replace @str with the new string
 * and the sx pointer in the ruby object with the new one.  Must make
 * sure the hack to store the new sx pointer follows the rules for
 * playing nicely with the garbage collector
 */
static VALUE sexp_unparse(VALUE self) {
  /* get the ary */
  VALUE ary = rb_iv_get(self,"@ary");
  /* turn ary into a sexp_t */
  sexp_t *sx = sexp_unparse_array(ary);
  /* the CSTRING we unparse sx into, the ruby string it represents, and
   * the old sx pointer to replace
   */
  CSTRING *s = NULL;
  VALUE str;
  sexp_t *sx_old;

  /* unparse sx */
  print_sexp_cstr(&s, sx, 256,128);
  /* make sure the CSTRING char * is null terminated */
  s->base[s->curlen] = '\0';
  /* create the ruby string */
  str = rb_str_new2(s->base);
  /* set the str field to the new string.  Make sure this is the correct
   * way to do it while still making sure the original we are replacing
   * gets garbage collected
   */
  rb_iv_set(self, "@str", str);

  /* assuming the string was copied in the rb_str_new2() call, dispose of 
   * the CSTRING.
   */
  sdestroy(s);
  
  /* look up the old sx pointer in the ruby object */
  Data_Get_Struct(self, sexp_t, sx_old);
  /* ...and destroy it */
  destroy_sexp(sx_old);

  /* stash the new sx pointer in the object */
  DATA_PTR(self) = (void *)sx;

  /* return self */
  return self;
}

/* setter for ary field
 */
static VALUE sexp_setAry(VALUE self, VALUE ary) {
  rb_iv_set(self, "@ary", ary);
  return self;
}

/* accessor for ary field
 */
static VALUE sexp_getAry(VALUE self) {
  return rb_iv_get(self, "@ary");
}

/* accessor for str field
 */
static VALUE sexp_getStr(VALUE self) {
  return rb_iv_get(self, "@str");
}

/* call made by ruby when loading the dynamlic library of this code.
 * defines the Sexp class and the methods on it.  They are implemented
 * above.
 */
void Init_Sexp() {
  cSexp = rb_define_class("Sexp", rb_cObject);
  rb_define_singleton_method(cSexp, "new", sexp_new, 1);
  rb_define_method(cSexp, "initialize", sexp_init, 2);
  rb_define_method(cSexp, "getAry", sexp_getAry, 0);
  rb_define_method(cSexp, "getStr", sexp_getStr, 0);
  rb_define_method(cSexp, "setAry", sexp_setAry, 1);
  rb_define_method(cSexp, "unparse", sexp_unparse, 0);
}
