/* $Id$
 * Generic Queue
 * Dino Dai Zovi <ghandi@cs.unm.edu>
 */

#ifndef QUE_H
#define QUE_H

struct que_elem {
    struct que_elem* next;
    struct que_elem* prev;

    void* data;
};

typedef struct {
    int valid;
    size_t size;
    struct que_elem* head;
    struct que_elem* tail;
} que_t;

#define QUE_VALID 0xdeadbeef
#define QUE_INITIALIZER {QUE_VALID, 0, 0}

extern int   que_init(que_t*);
extern int   que_destroy(que_t*);
extern int   que_ins(que_t*, void*);
extern void* que_rem(que_t*);
extern void* que_del(que_t*, void*);
extern int   que_empty(que_t*);

#endif
