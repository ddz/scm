/* $Id$ */
/*
 * Memory allocation and garbage collection
 */

#ifndef GC_H
#define GC_H

#include <inttypes.h>
#include <stdlib.h>

extern void gc_init(size_t heap_size);
extern void* gc_alloc(size_t size);

#endif
