/* $Id$ */
/*
 * Memory  allocation and garbage collection
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "scheme.h"

static size_t   heap_size;

static uint8_t* fromspace = NULL;
static uint8_t* tospace = NULL;

static uint8_t* scan = NULL;
static uint8_t* free_ptr = NULL;
static int      free_space = 0;

#define FORWARDED ((FORWARDPTR_T << 3) | 6)

static void gc_flip();

void gc_init(size_t size)
{
    size_t semi_space_size = size / 2;
    heap_size = size;
    
    fromspace = malloc(semi_space_size);
    tospace = malloc(semi_space_size);

    free_ptr = tospace;
    free_space = semi_space_size;
}

void* gc_alloc(size_t size)
{
    void* ptr;

    if (free_space - (int)size < 0) {
        fprintf(stderr, "Starting garbage collection...\n");
        gc_flip();
    }

    ptr = free_ptr;
    free_space -= size;
    free_ptr += size;

    return ptr;
}

// Copy scheme_t heap object to new space and return new scheme_t
scheme_t gc_copy(scheme_t s)
{
    if (!IS_HEAPPTR(s))
        return s;

    if (GET_CAR(GET_PTR(s)) == FORWARDED)
        return GET_CDR(GET_PTR(s));
    else {
        uint8_t* addr = free_ptr;
        memcpy(addr, (void*)GET_PTR(s), sizeof(cell_t));
        free_ptr = free_ptr + sizeof(cell_t);
        free_space -= sizeof(cell_t);
        
        GET_CAR(GET_PTR(s)) = FORWARDED;
        GET_CDR(GET_PTR(s)) = (scheme_t)addr;

        if (IS_PAIRPTR(s))
            return MAKE_PAIRPTR(addr);
        return MAKE_CELLPTR(addr);
    }
}

void gc_flip()
{
    int i;
    void* tmp = tospace;
    
    tospace = fromspace;
    fromspace = tmp;

    scan = free_ptr = tospace;
    free_space = heap_size / 2;

    // foreach root
    // top level environment, evaluator registers

    /*
     * Scan top level environment
     */
    for (i = 0; i < top_env->bindings->size; i++) {
        map_entry_t* e = top_env->bindings->table[i];
        while (e) {
            e->key = (void*)gc_copy((scheme_t)e->key);
            e->data = (void*)gc_copy((scheme_t)e->data);
            e = e->next;
        }
    }

    /*
     * Scan evaluator registers unless they are immediates
     */
    if (IS_HEAPPTR(expr)) 
        expr = gc_copy(expr);
    if (IS_HEAPPTR(rands))
        rands = gc_copy(rands);
    if (IS_HEAPPTR(val))
        val = gc_copy(val);
    if (IS_HEAPPTR(proc))
        proc = gc_copy(val);
    if (IS_HEAPPTR(args))
        args = gc_copy(val);

    // scan tospace for more objects to copy over
    
    while (scan < free_ptr) {
        cell_t* c = (cell_t*)scan;

        if ((GET_CAR(*c) & 7) == 6) {
            GET_CDR(*c) = gc_copy(GET_CDR(*c));
        }
        else {
            switch((GET_CAR(*c) >> 3) & 7) {
            case VECTOR_T: {
                scheme_t* v = (scheme_t*)GET_CDR(*c);
                int i, n = GET_CAR(*c) >> 6;
                for (i = 0; i < n; i++)
                    v[i] = gc_copy(v[i]);
            }
                
            case PROCEDURE_T: {
                struct procedure* p = (struct procedure*)GET_CDR(*c);
                if (p->type == COMPOUND) {
                    p->data.compound.formals =
                        gc_copy(p->data.compound.formals);
                    p->data.compound.body =
                        gc_copy(p->data.compound.body);
                }
            }
            }
        }

        scan += sizeof(cell_t);
    }
}
