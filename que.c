/* $Id$
 * Generic Queue Routines
 * Dino Dai Zovi <ghandi@cs.unm.edu>
 */

#include <stdlib.h>
#include <errno.h>
#include "que.h"

/*
 * que_init - Initialize a user-allocated queue.
 */
int que_init(que_t* q)
{
    q->head = q->tail = 0;
    q->valid = QUE_VALID;
    q->size = 0;
    
    return 0;
}

/*
 * que_destroy - destroy and free a queue.
 */
int que_destroy(que_t* q)
{
    struct que_elem* e;
    if (q->valid != QUE_VALID) {
	errno = EINVAL;
	return -1;
    }

    e = q->head;
    while (e) {
	struct que_elem* tmp = e;
	e = e->next;
	free(tmp);
    }

    return 0;
}

/*
 * que_ins - Insert a new element at the end of the queue.
 */
int que_ins(que_t* q, void* d)
{
    struct que_elem* n;
    
    if (q->valid != QUE_VALID) {
	errno = EINVAL;
	return -1;
    }

    n = malloc(sizeof(struct que_elem));
    n->prev = q->tail;
    n->next = 0;
    n->data = d;
    
    if (q->tail)
	q->tail->next = n;
    else
	q->head = n;
    q->tail = n;

    q->size++;
    
    return 0;
}

/*
 * que_rem - Remove element from the front of the queue.
 */
void* que_rem(que_t* q)
{
    struct que_elem* e;
    void* d;

    if (q->valid != QUE_VALID) {
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
 * que_del - Remove entry containing the specified data
 */
void* que_del(que_t* q, void* d)
{
    struct que_elem* i;

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
 * que_empty - return 1 if the queue is empty, 0 otherwise.
 */
int que_empty(que_t* q)
{
    if (q->valid != QUE_VALID) {
	errno = EINVAL;
	return -1;
    }
    
    return (!(q->head || q->tail));
}
