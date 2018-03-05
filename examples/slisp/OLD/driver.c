/*
 * Filename: driver.c
 * Author  : matt@lanl.gov
 * Created : 19 Mar 2003
 */
#include <stdio.h>
#ifndef WIN32
# include <unistd.h>
#else
# define ssize_t int
# include <io.h>
# include <sys/types.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include "sexp.h"
#include "slisp.h"

int main(int argc, char **argv) {
  char buf[BUFSIZ], outbuf[BUFSIZ];
  sexp_t *in, *out;
  sexp_iowrap_t *iow;
  int fd;

  fd = open("test_expressions",O_RDONLY);
  iow = init_iowrap(fd);

  in = read_one_sexp(iow);
  while (in != NULL) {
    printf("\n---------------------------------------------\n");
    print_sexp(buf,BUFSIZ,in);
    printf("Evaluating : %s\n",buf);
    out = slisp_eval(in);
    if (out != NULL) {
      print_sexp(outbuf,BUFSIZ,out);
      printf("%s ==> %s\n",buf,outbuf);
    } else {
      printf("%s ==> ERROR\n",buf);
    }

#ifdef _DEBUG_
    printf("\n");
#endif /* _DEBUG_ */

    destroy_sexp(in);
    destroy_sexp(out);
    in = read_one_sexp(iow);
  }

  destroy_iowrap(iow);

  exit(EXIT_SUCCESS);
}
