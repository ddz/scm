/* $Id$ */
/*
 * Primitive Scheme procedures relating to variable types.
 */

#include "types.h"

scheme_t make_symbol(char* name, size_t len)
{
    scheme_t s = MAKE_CELL();
    SCHEME_SET_CARX(s, (len << 5) | (SYMBOL_T << 3) | 6);
    SCHEME_SET_CDRX(s, name);
    return s;
}

scheme_t make_string(char* name, size_t len)
{
    scheme_t s = MAKE_CELL();
    SCHEME_SET_CARX(s, (len << 5) | (STRING_T << 3) | 6);
    SCHEME_SET_CDRX(s, name);
    return s;
}

scheme_t scheme_cons(scheme_t car, scheme_t cdr)
{
    scheme_t s = MAKE_PAIR();
    SCHEME_SET_CARX(s, car);
    SCHEME_SET_CDRX(s, cdr);
    return s;
}
