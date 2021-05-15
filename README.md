sfsexp
======

This library is intended for developers who wish to manipulate (read, parse, modify, and create) symbolic expressions from C or C++ programs. A symbolic expression, or s-expression, is essentially a LISP-like expression such as (a (b c)). S-expressions are able to represent complex, structured data without requiring additional meta-data describing the structure. They are recursively defined: an s-expression is a list of either atoms or s-expressions. In the example above, the expression contains an atom "a" and an s-expression, which in turn contains two atoms, "b" and "c". They are simple, useful, and well understood.

This library is designed to provide a minimal set of functions and data structures for the four functions listed above: reading s-expressions (I/O), parsing strings containing them into an AST equivalent, modifying the AST representation, and converting the AST back into a well formatted string. The primary goals are efficiency and simplicity. This library forms the basis of the data representation and transmission protocol for the supermon high-speed cluster monitoring system from the LANL ACL. The usefulness and lack of choice in available, open source s-expression libraries motivated the independent (from supermon) release of this library. Although the number of potential users represents a rather small community, the author felt it was a valuable contribution. As of March 2005, this library has actually received more interest in terms of downloads and page views than it's parent project!

This work was funded by the Department of Energy, Office of Science. The original development was performed at the Los Alamos National Laboratory between 2002 and 2007. It is currently maintained independently of any funding source as a service to the community (plus, it's fun to work on).

## Usage notes

The API is well-documented in the header files, esp.
[sexp.h](src/sexp.h) and [sexp_ops.h](src/sexp_ops.h). The library
includes a basic (optional) string library as well; see
[cstring.h](src/cstring.h).

If you are parsing a set of smallish sexps, as you might have in a
lispy source file, you probably want to use `init_iowrap` and
`read_one_sexp`. The [examples](examples) and [tests](tests)
directories contain multiple examples showing how to do this.

The drawback with `read_one_sexp` is that it uses a read buffer of
size `BUFSIZ`, which may relatively small (1024 on MacOS Big Sur). If
you try to read a sexp that is larger than `BUFSIZ` using this method
you will get error `SEXP_ERR_INCOMPLETE`, meaning "parsing is
incomplete and needs more data to complete it." This can easily happen
if you are reading data encoded as sexps. For example, it's not
uncommon for a big hunk o' data to be encoded a a single sexp in a
file. One way to handle this situation is to get the file size,
dynamically allocate a buffer big enought to hold it, and then use
`parse_sexp`. Here's a minimal example (error checking omitted):

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
     \/
    sexp_t  -- next --> sexp_t  -- next --> sexp_t
      |                   |                   |
     val                 list                val
      |                   |                   |
     \/                   |                  \/
      a                   |                   d
                         \/
                        sexp_t --> next --> sexp_t
                          |                   |
                         val                 val
                          |                   |
                         \/                  \/
                          b                   c
```
