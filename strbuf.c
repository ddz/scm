/* $Id$ */
/*
 * An infinitely (as available memory allows) growing string buffer.
 */

#include <stdio.h>
#include <errno.h>

#define STRBUF_VALID 0xfeedface
#define STRBUF_INITIALIZER {STRBUF_VALID, 0, 0, 0}

typedef struct {
    int    magic;   /* Magic number   */
    char*  buf;     /* String buffer  */
    size_t size;    /* Size of buffer */
    size_t len;     /* Length used    */
} strbuf_t;

int strbuf_init(strbuf_t* sb)
{
    sb->magic = STRBUF_VALID;
    sb->buf = NULL;
    sb->size = sb->len = 0;

    return 0;
}

int strbuf_destroy(strbuf_t* sb)
{
    if (sb->magic != STRBUF_VALID) { errno = EINVAL; return -1; }

    free(sb->buf);

    return 0;
}

int strbuf_reset(strbuf_t* sb)
{
    if (sb->magic != STRBUF_VALID) { errno = EINVAL; return -1; }

    free(sb->buf);
    sb->buf = NULL;
    sb->size = sb->len = 0;

    return 0;
}

int strbuf_append(strbuf_t* sb, char* s)
{

}

int strbuf_length(strbuf_t* sb)
{

}
