/* $Id$ */
/*
 * 6.6.1 Ports
 */

#include <stdio.h>
#include "scheme.h"

scheme_t make_port(FILE* f)
{
    scheme_t s = MAKE_CELL();

    GET_CAR(GET_PTR(s)) = (PORT_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)f;

    return s;
}

scheme_t scheme_read_char_1(scheme_t port)
{
    FILE* f = (FILE*)scheme_cdr(port);
    return MAKE_CHAR(getc(f));
}

scheme_t scheme_peek_char_1(scheme_t port)
{
    char c;
    FILE* f = (FILE*)scheme_cdr(port);
    c = getc(f);
    ungetc(c, f);
    return MAKE_CHAR(c);
}

scheme_t scheme_load(scheme_t filename)
{
    char* fn;
    FILE* f;
    scheme_t s, e;
    
    if (!IS_STRING(filename))
        error("load: expects string\n");

    fn = GET_STRING(filename);
    f = fopen(fn, "r");

    if (!f)
        error("load: couldn't open file\n");

    while ((s = scheme_read(f)) != SCHEME_EOF) {
        e = scheme_eval(s, top_env);
        scheme_write_1(e);
    }

    return SCHEME_UNSPEC;
}
