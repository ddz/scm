/* $Id$
 * Generic Stack
 * Dino Dai Zovi <ghandi@cs.unm.edu>
 */

#ifndef STK_H
#define STK_H

struct stk_elem {
    struct stk_elem* next;
    struct stk_elem* prev;

    void* data;
};

typedef struct {
    int valid;
    size_t size;
    struct stk_elem* head;
    struct stk_elem* tail;
} stk_t;

#define STK_VALID 0xdeadbeef
#define STK_INITIALIZER {STK_VALID, 0, 0}

extern int   stk_init(stk_t*);
extern int   stk_destroy(stk_t*);
extern int   stk_push(stk_t*, void*);
extern void* stk_pop(stk_t*);
extern void* stk_top(stk_t*);
extern void* stk_del(stk_t*, void*);
extern int   stk_empty(stk_t*);

#endif
