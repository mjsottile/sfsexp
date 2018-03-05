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

/**
 * memory management related stuff for slisp
 * based on Erik Hendriks' malloc debugging stuff that matt can't find now
 */
#ifndef __SLISP_MEMMAN_H__
#define __SLISP_MEMMAN_H__

/* structures for tracking leaks */
struct allocated_memory {
  unsigned int  address;
  unsigned int  line;
  char         *filename;
  struct allocated_memory *prev;
  struct allocated_memory *next;

};

#ifdef __cplusplus
extern "C" {
#endif

  void reset_memory_counters();
  void report_memory_counters();
  void add_mem_checkpoint(unsigned int addr, unsigned int line, char *fname);
  void free_mem_checkpoint(void *ptr, int line, char *fname);

#ifdef __cplusplus
}
#endif

/* macro to hide uglier call with line and file stuff */
#define MEM_CHECKPOINT(ptr) add_mem_checkpoint((unsigned int)(ptr),__LINE__,__FILE__);
#define FREE_CHECKPOINT(ptr) free_mem_checkpoint((ptr),__LINE__,__FILE__);

#endif /* __SLISP_MEMMAN_H__ */
