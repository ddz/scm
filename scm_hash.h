/* $Id$ */
/*
 * Scheme scheme_t->scheme_t Hash Table used for environment bindings
 * and as a general purpose Scheme data structure.
 */

#ifndef SCM_HASH_H
#define SCM_HASH_H

#include <sys/types.h>

#define SCM_HASH_VALID 0xcafebabe

typedef struct {
    int       magic;
    size_t    used;
    size_t    size;
    float     rehash_size;
    float     rehash_threshold;
    scheme_t* table;
} scm_hash_t;

#endif
