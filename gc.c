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
    scheme_t* c = (scheme_t*)GET_PTR(s);

    assert(IS_HEAPPTR(s));
    
    if (c[0] == FORWARDED)
        return c[1];
    else {
	scheme_t f;
        uint8_t* addr = free_ptr;
	
        memcpy(addr, c, sizeof(cell_t));
        free_ptr += sizeof(cell_t);
        free_space -= sizeof(cell_t);

	if (IS_PAIRPTR(s))
	    f = MAKE_PAIRPTR(addr);
	else
	    f = MAKE_CELLPTR(addr);
	
        c[0] = FORWARDED;
        c[1] = f;

        return f;
    }
}

void trace_env(env_frame_t* env)
{
    int i;
    for (i = 0; i < env->bindings->size; i++) {
        map_entry_t* e = env->bindings->table[i];
        while (e) {
            if (IS_HEAPPTR((scheme_t)e->key))
                e->key = (void*)gc_copy((scheme_t)e->key);
            if (IS_HEAPPTR((scheme_t)e->data))
                e->data = (void*)gc_copy((scheme_t)e->data);
            e = e->next;
        }
    }

    if (env->env)
        trace_env(env);
}

void gc_flip()
{
    continuation_t* c;
    void* tmp = tospace;
    
    tospace = fromspace;
    fromspace = tmp;

    scan = free_ptr = tospace;
    free_space = heap_size / 2;

    /*
     * Scan top level environment
     */
    trace_env(top_env);
    
    /*
     * Scan evaluator registers unless they are immediates
     */
    if (env != top_env)
	trace_env(env);
    
    if (IS_HEAPPTR(expr)) 
        expr = gc_copy(expr);
    if (IS_HEAPPTR(rands))
        rands = gc_copy(rands);
    if (IS_HEAPPTR(val))
        val = gc_copy(val);
    if (IS_HEAPPTR(proc))
        proc = gc_copy(proc);
    if (IS_HEAPPTR(args))
        args = gc_copy(args);
    
    c = cont;
    while (c != NULL) {
	if (c->envt)
	    trace_env(c->envt);
	switch (c->type) {
	case TEST:
	    if (IS_HEAPPTR(c->data.test.true_expr))
		c->data.test.true_expr =
		    gc_copy(c->data.test.true_expr);
	    if (IS_HEAPPTR(c->data.test.false_expr))
		c->data.test.false_expr =
		    gc_copy(c->data.test.false_expr);
	    break;

	case VARASSIGN:
	case DEFINITION:
	    if (IS_HEAPPTR(c->data.assignment.var))
		c->data.assignment.var =
		    gc_copy(c->data.assignment.var);
	    break;

	case BEGIN:
	    if (IS_HEAPPTR(c->data.eval_begin.exprs))
		c->data.eval_begin.exprs =
		    gc_copy(c->data.eval_begin.exprs);
	    break;
	    
	case EVAL_RATOR:
	    if (IS_HEAPPTR(c->data.eval_rator.rands))
		c->data.eval_rator.rands =
		    gc_copy(c->data.eval_rator.rands);
	    break;
	    
	case EVAL_RANDS:
	    if (IS_HEAPPTR(c->data.eval_rands.proc))
		c->data.eval_rands.proc =
		    gc_copy(c->data.eval_rands.proc);
	    break;
	    
	case EVAL_FIRST:
	    if (IS_HEAPPTR(c->data.eval_first.exprs))
		c->data.eval_first.exprs =
		    gc_copy(c->data.eval_first.exprs);
	    break;
	    
	case EVAL_REST:
	    if (IS_HEAPPTR(c->data.eval_rest.first_value))
		c->data.eval_rest.first_value =
		    gc_copy(c->data.eval_rest.first_value);
	    break;

	default:
	    fprintf(stderr, "Unknown continuation type\n");
	    abort();
	}

	c = c->cont;
    }
    
    // scan tospace for more objects to copy over
    
    while (scan < free_ptr) {
        scheme_t* s = (scheme_t*)scan;

        if ((s[0] & 7) == 6) {
            switch((s[0] >> 3) & 7) {
            case VECTOR_T: {
                scheme_t* v = (scheme_t*)s[1];
                int i, n = s[0] >> 6;
                for (i = 0; i < n; i++) {
                    if (IS_HEAPPTR(v[i]))
                        v[i] = gc_copy(v[i]);
                }
		break;
            }
                
            case PROCEDURE_T: {
                struct procedure* p = (struct procedure*)s[1];
                if (p->type == COMPOUND) {
                    // scan the env
                    trace_env(p->data.compound.env);
                    if (IS_HEAPPTR(p->data.compound.formals))
                        p->data.compound.formals =
                            gc_copy(p->data.compound.formals);
                    if (IS_HEAPPTR(p->data.compound.body))
                        p->data.compound.body =
                            gc_copy(p->data.compound.body);
                }
		break;
            }

	    case FORWARDPTR_T:
		fprintf(stderr, "forwardptr found in scan\n");
		abort();
	    
            }
        }
	else {
	    if (IS_HEAPPTR(s[0]))
		s[0] = gc_copy(s[0]);
	    if (IS_HEAPPTR(s[1]))
		s[1] = gc_copy(s[1]);
	}

        scan += sizeof(cell_t);
    }
}
