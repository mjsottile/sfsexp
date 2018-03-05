/*****************************************************************************
 *
 * pysexp - A simple python binding to libsexp to translate an S expression
 *	into a complex python data structure
 *
 * Copyright 2004, Steven James <pyro@linuxlabs.com> and
 *	Linux Labs International http://www.linuxlabs.com
 *
 * Thanks to Matt Sottile <matt@lanl.gov> for the small, fast s-expression library
 * but don't blame him for bugs in the Python binding :-)
 *
 * This work is released under the terms of the General Public License version
 * 2 or later.
 *
 ****************************************************************************/

#include <Python.h>
#include <sexp.h>

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define DPRINTF( X... ) fprintf(stderr, X)
#else
#define DPRINTF( X... ) {}
#endif

struct sexp_parser_state_t *state;
struct sexp_t *out;

static PyObject *iter(sexp_t *s);
static PyObject *iter_list(sexp_t *s);

static PyObject *iter_val(sexp_t *s)
{
	PyObject *ret;

	return PyString_FromString( s->val);

}

/****************************************************************************
 *
 * Translation functions: These iterate through a parsed S-expression and 
 * translate it into a complex hierarchy of python objects.
 *
 * Due to the richer variety of Python objects, a few conventions are adopted.
 *
 * Lists containing only one value are stripped:
 * ( foo ) -> foo
 *
 * lists containing 2 items become a dict:
 * ( foo bar ) -> { foo: bar }
 * ( foo bar baz ) -> { foo: [bar baz] }
 *
 * Lists containing only dicts are merged into a single dict:
 * ( ( foo 1 ) ( bar 2 ) ( baz 5 6 7 8) )
 * -> { foo:1, bar:2, baz:[ 5 6 7 8] }
 *
 * ((( foo bar ))) -> { foo:bar }
 *
 *****************************************************************************/

// Special list handling
// If S has length 1, 'promote' it to a value
// If S starts with a value, 'promote' it to a dict.

//#define Py_OneRef( x) { fprintf(stderr, "obj: %p, refcnt = %u\n", x, x->ob_refcnt); if(x->ob_refcnt) x->ob_refcnt=2; }
#define Py_OneRef( x) Py_DECREF(x)

static PyObject *iter_real_list(sexp_t *s)
{
	PyObject *list;
	PyObject *ret;
	sexp_t *r = s;
	char alldict=1;

	if(!s->next) {
		if( s->ty == SEXP_LIST)
			return( iter_list(s->list));
		return iter_val(s);
	}

	list = PyList_New(0);
	while(r) {
		ret = iter(r);
		if(! PyDict_Check(ret))
			alldict = 0;
		PyList_Append( list, ret);
		Py_OneRef(ret);
		r = r->next;
	}

	if(alldict) {
		PyObject *dict, *subdict;
		int i, count;

		dict = PyDict_New();
		count = PyList_GET_SIZE(list);
		for(i=0; i<count; i++) {
			subdict = PyList_GET_ITEM( list, i);
			PyDict_Merge( dict, subdict, 0);
//			Py_DECREF(subdict);
		}

		Py_DECREF(list);
		return(dict);
	}

	return list;
}

static PyObject *iter_list(sexp_t *s)
{
	PyObject *list;
	PyObject *ret;
	PyObject *dict2;
	sexp_t *r = s;
	char alldict=1;

	if(!s)
		return( PyList_New(0));

	if(!s->next) {
		if( s->ty == SEXP_LIST)
			return( iter_list(s->list));
		return iter_val(s);
	}

	if(s->ty == SEXP_VALUE) {
		list= PyDict_New();

		ret = iter_real_list( s->next);
		PyDict_SetItemString( list, s->val, ret);
		Py_OneRef(ret);

		return list;
	}
	

	list = PyList_New(0);
	while(r) {
		ret = iter(r);
		PyList_Append( list, ret);
		Py_OneRef(ret);
		r = r->next;
	}

	if(alldict) {
		PyObject *dict, *subdict;
		int i, count;

		dict = PyDict_New();
		count = PyList_GET_SIZE(list);
		for(i=0; i<count; i++) {
			subdict = PyList_GET_ITEM( list, i);
			PyDict_Merge( dict, subdict, 0);
//			Py_DECREF(subdict);
		}

		Py_DECREF(list);
		return(dict);
	}
	return(list);
}

static PyObject *iter(sexp_t *s)
{
	PyObject *ret;
	PyObject *subret;

	if(s->ty == SEXP_LIST) 
		ret = iter_list( s->list );
	else
		ret = iter_val( s);

	return ret;

}

static char parse__doc__[] =
	"parse() - Accepts an S-expression in a string, returns a complex Python structure.\n";

static PyObject *pysexp_parse( PyObject *self, PyObject *args)
{
	PyObject *list, *list2;
	char *in;
	sexp_t *out;
	int ret;

	if(!PyArg_ParseTuple( args, "s", &in))
		return;

	out = parse_sexp( in, strlen(in));
	if(!out) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	DPRINTF("parsing: %s\n", in);

	list = iter(out);

	DPRINTF("list = %p\n", list);
	destroy_sexp(out);

	return list;
}

/* functions supporting continuations */

typedef struct ContFind {
	int handle;

	sexp_t *sx;
	struct ContFind *prev;
	struct ContFind *next;
} ContFind_t;

typedef struct ContSexpr {
	int handle;
	
	pcont_t *pc;
	sexp_t *sx;

	ContFind_t *finds;
	struct ContSexpr *prev;
	struct ContSexpr *next;
} ContSexpr_t;

static PyMethodDef PySexpMethods[] = {
	{"parse", pysexp_parse, METH_VARARGS, parse__doc__},
    {NULL,NULL},
};

void initpysexp(void)
{
	PyObject *m, *d, *i;

	m = Py_InitModule("pysexp", PySexpMethods);
	d = PyModule_GetDict(m);
}

