/* $Id$ */
/*
 * Generic Stack Routines
 *
 * Dino Dai Zovi <ghandi@cs.unm.edu>
 */

#include <stdlib.h>
#include <errno.h>
#include "stk.h"

/*
 * stk_init - Initialize a user-allocated stack.
 */
int stk_init(stk_t* q)
{
    q->head = q->tail = 0;
    q->valid = STK_VALID;
    q->size = 0;
    
    return 0;
}

/*
 * stk_destroy - destroy and free a stack.
 */
int stk_destroy(stk_t* q)
{
    struct stk_elem* e;
    if (q->valid != STK_VALID) {
	errno = EINVAL;
	return -1;
    }

    e = q->head;
    while (e) {
	struct stk_elem* tmp = e;
	e = e->next;
	free(tmp);
    }

    return 0;
}

/*
 * stk_push - Insert a new element at the top of the stack.
 */
int stk_push(stk_t* q, void* d)
{
    struct stk_elem* n;
    
    if (q->valid != STK_VALID) {
	errno = EINVAL;
	return -1;
    }

    n = malloc(sizeof(struct stk_elem));
    n->prev = 0;
    n->next = q->head;
    n->data = d;

    if (q->head)
	q->head->prev = n;
    if (q->tail == NULL)
	q->tail = n;
    
    q->head = n;    
    q->size++;
    
    return 0;
}

/*
 * stk_pop - Remove element from the front of the stack.
 */
void* stk_pop(stk_t* q)
{
    struct stk_elem* e;
    void* d;

    if (q->valid != STK_VALID) {
	errno = EINVAL;
	return 0;
    }

    e = q->head;
    d = e->data;
    q->head = e->next;
    free(e);
    
    if (q->head == NULL)
	q->tail = NULL;
    else
	q->head->prev = NULL;

    q->size--;
    
    return d;
}

/*
 * stk_top - Return top-most element on stack.
 */
void* stk_top(stk_t* q)
{
    struct stk_elem* e;
    void* d;

    if (q->valid != STK_VALID) {
        errno = EINVAL;
        return 0;
    }

    return q->head->data;
}

/*
 * stk_del - Remove entry containing the specified data
 */
void* stk_del(stk_t* q, void* d)
{
    struct stk_elem* i;

    for (i = q->head; i != NULL; i = i->next) {
	if (*(char*)i->data == *(char*)d) {
	    void* tmp_data;
	    
	    if (i == q->head)
		q->head = i->next;
	    if (i == q->tail)
		q->tail = i->prev;

	    if (i->next)
		i->next->prev = i->prev;
	    if (i->prev)
		i->prev->next = i->next;

	    tmp_data = i->data;
	    free(i);
	    q->size--;
	    
	    return tmp_data;
	}
    }
    
    return NULL;
}

/*
 * stk_empty - return 1 if the stack is empty, 0 otherwise.
 */
int stk_empty(stk_t* q)
{
    if (q->valid != STK_VALID) {
	errno = EINVAL;
	return -1;
    }
    
    return (!(q->head || q->tail));
}
