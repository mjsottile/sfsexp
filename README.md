sfsexp
======

This library is intended for developers who wish to manipulate (read, parse, modify, and create) symbolic expressions from C or C++ programs. A symbolic expression, or s-expression, is essentially a LISP-like expression such as (a (b c)). S-expressions are able to represent complex, structured data without requiring additional meta-data describing the structure. They are recursively defined: an s-expression is a list of either atoms or s-expressions. In the example above, the expression contains an atom "a" and an s-expression, which in turn contains two atoms, "b" and "c". They are simple, useful, and well understood.

This library is designed to provide a minimal set of functions and data structures for the four functions listed above: reading s-expressions (I/O), parsing strings containing them into an AST equivalent, modifying the AST representation, and converting the AST back into a well formatted string. The primary goals are efficiency and simplicity. This library forms the basis of the data representation and transmission protocol for the [Supermon](https://dl.acm.org/doi/10.5555/792762.793324) high-speed cluster monitoring system from the LANL Advanced Computing Laboratory. The usefulness and lack of choice in available, open source s-expression libraries around 2003 motivated the independent (from supermon) release of this library. 

## Building

(Attention Windows Users: If you are using cygwin, this section applies as
cygwin looks pretty much like unix for compilation.  Visual Studio users see
the note at the end and look inside the win32 directory.)

Configure the sources via autoconf.

```
% ./configure
```

Currently, the only feature that can be enabled via autoconf is to enable any
debugging code in the library by specifying "--enable-debug".  Other features
such as disabling memory management by the library are toggled by setting
appropriate options in the CFLAGS:

```
% CFLAGS=-D_NO_MEMORY_MANAGEMENT_ ./configure
```
Note that you should use the vanilla configuration unless you know that you
want to use debug mode or other options, and understand precisely what they
mean.

After building, just invoke make:

```
% make
```

What comes out is a library called "libsexp.a".  If you wish to copy the
headers and libraries to an installation location, you should then say:

```
% make install
```

At the current time, this is not recommended if the installation prefix is
/usr or /usr/local, since the headers do not go into an isolated subdirectory.
In the future, we will hopefully have a single consolidated header that
encapsulates all of the headers currently included in the library.  If you do
not want to aim your project that uses the library into the library source and
build directories directly, creating a separate installation prefix and
installing there is recommended.  For example, from the root of the sexpr
tree:

```
% mkdir installTree
% ./configure --prefix=`pwd`/installTree
% make
% make install
```

If you want the docs, make sure you have doxygen installed and that the
DOXYGEN variable in the Makefile.in in this directory points at the right
place.  Re-run autoconf to regenerate the makefiles, and then type:

```
% make doc
```

## Usage notes

In any code that wants to use this, just include "sexp.h".  That contains the
one data structure, enumeration, and five functions for manipulating and
parsing strings and s-expressions.  Compilation typically will look like:

```
% cc -I/path/to/sexp/include -L/path/to/sexp/library \
     -o foo  foo.o -lsexp
```

The critical parts are to ensure that the include path aims at the path
containing the library headers, the library path aims at the path containing
the compiled binary libraries, and the library is linked in with the
executable.

The API is well-documented in the header files, esp.
[sexp.h](src/sexp.h) and [sexp_ops.h](src/sexp_ops.h). The latter
contains some convenience operators that may make your life slightly
easier; for example, it defines `hd_sexp`, `tl_sexp`, and `next_sexp`,
which make it a little easier to navigate sexps. (see below for a
schematic representation of sexp structure).

The library includes a basic (optional) string library as well; see
[cstring.h](src/cstring.h).  This string library is useful to avoid
working with fixed sized buffers and will grow strings as necessary
automatically.

If you are parsing a set of smallish sexps, as you might have in a
lispy source file, you probably want to use `init_iowrap` and
`read_one_sexp`. The [examples](examples) and [tests](tests)
directories contain multiple examples showing how to do this.

The drawback with `read_one_sexp` is that it uses a read buffer of
size `BUFSIZ`, which may relatively small (1024 on MacOS Big Sur). If
you try to read a sexp that is larger than `BUFSIZ` using this method
you will get error `SEXP_ERR_INCOMPLETE`, meaning "parsing is
incomplete and needs more data to complete it."
([sexp_errors.h](src/sexp_errors.h)). This can easily happen if you
are reading data encoded as sexps. For example, it's not uncommon for
a big hunk o' data to be encoded as a single sexp in a file. One way to
handle this situation is to get the file size, dynamically allocate a
buffer big enought to hold it, and then use `parse_sexp`. Here's a
minimal example (error checking omitted):

```
int fd;
FILE *fp = fopen(fname, "r");
fseek(fp, 0, SEEK_END);
size_t fsize = (size_t) ftell(fp);
fseek(fp, 0, SEEK_SET);  /* reset file pos to beginning of file, for reading */
char *work_buf = (char*) malloc(fsize + 1);
char *check_buf = (char*) malloc(fsize + 1); /* for debugging */
size_t read_len = fread(work_buf, 1, fsize, fp);
work_buf[read_len] = '\0';       /* make sure it's properly terminated */
sexp_t the_sexp = parse_sexp(work_buf, read_len); /* assumption: file contains one sexp */
/* check the parse result by serializing it to check_buf */
size_t write_len = print_sexp(check_buf, fsize, the_sexp);
printf("sexp: '%s'", check_buf);
/* process the_sexp ... */
destroy_sexp(the_sexp);
free(work_buf);
free(check_buf);
close(fd);
```

The internal representation of sexps is lispy. For example, `(a (b c) d)`
looks something like this:

```
    sexp_t
      |
    list
      |
     \_/
    sexp_t  -- next --> sexp_t  -- next --> sexp_t
      |                   |                   |
     val                 list                val
      |                   |                   |
     \_/                  |                  \_/
      a                   |                   d
                          |
                         \_/
                        sexp_t --> next --> sexp_t
                          |                   |
                         val                 val
                          |                   |
                         \_/                 \_/
                          b                   c
```

## Windows users

Please look in the win32/ subdirectory. Note that as of 9/2013, this has not
been looked at nor tested in a number of years.  If you try to use it and find
it broken, fixes and updates to bring it up to speed with modern Windows
development environments would be appreciated!

## Credits

The library is by Matt Sottile.  Steve James of Linux Labs has contributed bug
fixes and features while developing for the related Supermon project. Sung-Eun
Choi and Paul Ruth have contributed many bug reports as the library has grown.
Erik Hendriks contributed the malloc debugging tools now used when building
with the -D_DEBUG_MALLOCS_ option.  Brad Green contributed code (in win32/)
and testing for the Windows Visual Studio build target.  Others who have
contributed can be found in the Github repository contributor list.

### Funding acknowledgement

This work was funded by the Department of Energy, Office of Science. The original development was performed at the Los Alamos National Laboratory between 2002 and 2007. It is currently maintained independently of any funding source as a service to the community (plus, it's fun to work on).

