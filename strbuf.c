/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "strbuf.h"

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

int strbuf_add(strbuf_t* sb, char c)
{
    if (sb->len + 1 > sb->size) {
        while (sb->len + 1 < sb->size) {
            if (sb->size == 0)
                sb->size = 1;
            else
                sb->size = sb->size * 2;
        }
        sb->buf = realloc(sb->buf, sb->size);
    }

    sb->buf[sb->len++] = c;
    sb->buf[sb->len] = '\0';

    return 0;
}

int strbuf_append(strbuf_t* sb, char* s, size_t len)
{
    if (sb->len + len > sb->size) {
        while (sb->len + len < sb->size) {
            if (sb->size == 0)
                sb->size = 1;
            else
                sb->size = sb->size * 2;
        }
        sb->buf = realloc(sb->buf, sb->size);
    }

    strncat(sb->buf, s, len);
    sb->len += len;

    return 0;
}

size_t strbuf_length(strbuf_t* sb)
{
    return sb->len;
}

char* strbuf_buffer(strbuf_t* sb)
{
    return sb->buf;
}
