/* $Id$ */
/*
 * A data structure supporting generic keyed data storage.
 *
 * Dino Dai Zovi <ghandi@cs.unm.edu>
 */

#ifndef MAP_H
#define MAP_H

#include <stddef.h>

typedef size_t (*map_hash_t)(const void*);
typedef int    (*map_cmp_t)(const void*, const void*);

typedef struct _entry {
    void*          key;
    void*          data;
    struct _entry* next;
} map_entry_t;

#define MAP_VALID 0xcafebabe

typedef struct {
    unsigned long valid;
    
    /*
     * Hash table operations
     */
    map_hash_t hash_op;    /* Key hashing operation    */
    map_cmp_t  cmp_op;     /* Key comparison operation */
    
    /*
     * Hash table parameters
     */
    float rehash_threshold;
    struct {
        enum {SIZE, RATIO} type;
        union {
            size_t size;
            float  ratio;
        } rehash;
    } rehash_size;

    size_t size;
    size_t used;
    map_entry_t** table;
    
} map_t;

extern int    map_init(map_t*, map_hash_t, map_cmp_t);
extern int    map_put(map_t*, void*, void*);
extern void*  map_get(map_t*, const void*, int*);
extern map_entry_t** map_lookup(map_t* h, const void* key);

#endif
