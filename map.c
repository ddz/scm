/* $Id$ */
/*
 * Generic map implemented as a hash table
 */

#include <stdio.h>
#include <stdlib.h>
#include "map.h"

#define MAP_INIT_SIZE 1

static int map_resize(map_t* h, size_t size);
static map_entry_t** map_lookup(map_t* h, const void* key);

int map_init(map_t* h, map_hash_t hash_op, map_cmp_t cmp_op)
{
    h->valid = MAP_VALID;
    h->hash_op = hash_op;
    h->cmp_op = cmp_op;

    h->rehash_threshold = 0.5;
    h->rehash_size.type = RATIO;
    h->rehash_size.rehash.ratio = 2.0;

    h->size = 0;
    h->used = 0;
    h->table = NULL; // malloc(h->size * sizeof(map_entry_t*));
}

int map_resize(map_t* h, size_t size)
{
    int i, old_size = h->size;
    map_entry_t** old_table = h->table;
    
    h->size = size;
    if (!(h->table = malloc(h->size * sizeof(map_entry_t*)))) {
        fprintf(stderr, "map_resize: unable to allocate memory\n");
        return -1;
    }

    for (i = 0; i < old_size; i++) {
        if (old_table[i]) {
            map_entry_t* e = old_table[i];

            while (e) {
                map_entry_t*  f = e->next;
                map_entry_t** pe = map_lookup(h, e->key);
                *pe = e;
                e->next = NULL;
                e = f;
            }
        }
    }

    free(old_table);

    return 0;
}

map_entry_t** map_lookup(map_t* h, const void* key)
{
    unsigned int i;
    map_entry_t** e = NULL;

    if (h->table == NULL)
        return NULL;
    
    i = (*h->hash_op)(key) % h->size;

    for (e = &h->table[i]; *e; e = &(*e)->next) {
        if ((*h->cmp_op)(key, (*e)->key) == 0)
            return e;
    }

    return e;
}

void* map_get(map_t*h, const void* key)
{
    map_entry_t** e = map_lookup(h, key);
    if (e == NULL || *e == NULL)
        return NULL;
    else
        return (*e)->data;
}

int map_put(map_t* h, void* key, void* data)
{
    map_entry_t** e;
    
    if (h->table == NULL)
        map_resize(h, MAP_INIT_SIZE);

    e = map_lookup(h, key);
    
    if (*e == NULL) {
        *e = malloc(sizeof(map_entry_t));
        (*e)->key = key;
        (*e)->data = data;
        (*e)->next = NULL;

        if (++h->used > (h->rehash_threshold * h->size)) {
            size_t newsize;
            if (h->rehash_size.type == SIZE)
                newsize = h->size + h->rehash_size.rehash.size;
            else
                newsize = h->size * h->rehash_size.rehash.ratio;
            map_resize(h, newsize);
        }
        
        return 0;
    }

    return -1;
}

unsigned int map_hash(const void* key)
{
    /*
     * Default hashing algorithm is to return the value of the
     * pointer.  This works for objects unless they move around.
     */
    return (unsigned int)key;
}
