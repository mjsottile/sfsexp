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

#include "slisp_memman.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef __DISABLE_MEMMAN__
/* reference counting - used for debugging, disabled for production build */
static unsigned int malloc_count, free_count;

static struct allocated_memory *mem_allocated;
#endif


#ifdef __DISABLE_MEMMAN__
#define reset_memory_counters() {/*'da void*/} 
#else
void reset_memory_counters() {
  struct allocated_memory *tmp;
  malloc_count = free_count = 0;

  if (mem_allocated != NULL) {
    for (tmp = mem_allocated; tmp != NULL; tmp = tmp->next) {
      free(tmp->filename);
      free(tmp);
    }
    mem_allocated = NULL;
  }
}
#endif

#ifdef __DISABLE_MEMMAN__
#define report_memory_counters() { /* nada */ }
#else
void report_memory_counters() {
  struct allocated_memory *tmp;
  int i;
  
  if (malloc_count == free_count &&
      mem_allocated == NULL) {
    return;
  }

  for (i=0;i<72;i++) fprintf(stderr,"="); fprintf(stderr,"\n");

  fprintf(stderr,"MALLOCS/FREES = [%d / %d]\n",malloc_count,free_count);
  tmp = mem_allocated;
  if (tmp != NULL) {
    for (; tmp != NULL; tmp = tmp->next) {
      fprintf(stderr,"ADDRESS=0x%x :: ALLOCATED %s:%d\n",tmp->address,
              tmp->filename,tmp->line);
    }
  }

  for (i=0;i<72;i++) fprintf(stderr,"="); fprintf(stderr,"\n");
  fprintf(stderr,"\n");
}
#endif

#ifdef __DISABLE_MEMMAN__
#define add_mem_checkpoint(a,b,c) { /* nuttin' */ }
#else
void add_mem_checkpoint(unsigned int addr, unsigned int line, char *fname) {
  struct allocated_memory *tmp = mem_allocated;

  if (tmp == NULL) {
    mem_allocated = 
      (struct allocated_memory *)malloc(sizeof(struct allocated_memory));
    mem_allocated->next = mem_allocated->prev = NULL;
    mem_allocated->line = line;
    mem_allocated->address = addr;
    mem_allocated->filename = 
      (char *)malloc(sizeof(char)*(strlen(fname) + 1));
    assert(mem_allocated->filename != NULL);
    strcpy(mem_allocated->filename,fname);
  } else {
    for ( ; tmp->next != NULL; tmp = tmp->next) { }

    tmp->next = 
      (struct allocated_memory *)malloc(sizeof(struct allocated_memory));
    assert(tmp->next != NULL);
    tmp->next->prev = tmp;
    tmp = tmp->next;

    tmp->next = NULL;
    tmp->address = addr;
    tmp->line = line;
    tmp->filename = (char *)malloc(sizeof(char)*(strlen(fname) + 1));
    assert(tmp->filename != NULL);
    strcpy(tmp->filename,fname);
  }

  malloc_count++;
}
#endif

#ifdef __DISABLE_MEMMAN__
#define free_mem_checkpoint(a,b,c) { /* zilch */ }
#else
void free_mem_checkpoint(void *ptr, int line, char *filename) {
  unsigned int addr = (unsigned int)ptr;
  struct allocated_memory *tmp = mem_allocated;
  
  if (tmp == NULL) {
    fprintf(stderr,"WARNING!  Attempting to deallocate memory not recorded as allocated.\n");
    fprintf(stderr,"          Address = 0x%x\n",addr);
    fprintf(stderr,"          Checkpoint = %s:%d\n",filename,line);
  }

  for ( ; tmp != NULL; tmp = tmp->next) {
    if (tmp->address == addr)
      break;
  }

  if (tmp == NULL) {
    fprintf(stderr,"WARNING!  Attempting to deallocate memory not recorded as allocated.\n");
    fprintf(stderr,"          Address = 0x%x\n",addr);
    fprintf(stderr,"          Checkpoint = %s:%d\n",filename,line);
  } else {
    if (tmp->prev != NULL) tmp->prev->next = tmp->next;
    if (tmp->next != NULL) tmp->next->prev = tmp->prev;
    free(tmp->filename);
    if (tmp == mem_allocated && tmp->next == NULL)
      mem_allocated = NULL;
    free(tmp);
    free_count++;
  }
}
#endif
