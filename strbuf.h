/* $Id$ */
/*
 * An infinitely (as memory allows) growable string buffer.
 *
 * Dino Dai Zovi <ghandi@cs.unm.edu>
 */

#ifndef STRBUF_H
#define STRBUF_H

#include <sys/types.h>

#define STRBUF_VALID       0xfeedface
#define STRBUF_INITIALIZER {STRBUF_VALID, 0, 0, 0}

typedef struct {
    int    magic;   /* Magic number   */
    char*  buf;     /* String buffer  */
    size_t size;    /* Size of buffer */
    size_t len;     /* Length used    */
} strbuf_t;

extern int    strbuf_init(strbuf_t*);
extern int    strbuf_destroy(strbuf_t*);
extern int    strbuf_reset(strbuf_t*);
extern int    strbuf_append(strbuf_t*, char*, size_t);
extern size_t strbuf_length(strbuf_t*);
extern char*  strbuf_buffer(strbuf_t*);

#endif
