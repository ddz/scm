/* $Id$ */
/*
 * 6.6.1 Ports
 */

#include <stdio.h>
#include "scheme.h"

scheme_t make_port(FILE* f)
{
    scheme_t s = MAKE_CELL();
    scheme_set_carx(s, (PORT_T << 3) | 6);
    scheme_set_cdrx(s, f);
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
