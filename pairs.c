/* $Id$ */
/*
 * Scheme R5RS pairs and lists procedures
 */

#include <stdlib.h>
#include "scheme.h"

scheme_t scheme_pairp(scheme_t s)
{
    if (IS_PAIRPTR(s))
        return SCHEME_TRUE;
    else
        return SCHEME_FALSE;
}

scheme_t scheme_cons(scheme_t car, scheme_t cdr)
{
    scheme_t s = MAKE_PAIR();
    GET_CAR(GET_PTR(s)) = car;
    GET_CDR(GET_PTR(s)) = cdr;

    return s;
}

scheme_t scheme_car(scheme_t s)
{
    if (scheme_pairp(s) != SCHEME_TRUE)
        error("car: attempt to take car of non-pair");
    
    return GET_CAR(GET_PTR(s));
}

scheme_t scheme_cdr(scheme_t s)
{
    if (scheme_pairp(s) == SCHEME_FALSE)
        error("cdr: attempt to take cdr of non-pair");
    return GET_CDR(GET_PTR(s));
}

scheme_t scheme_set_carx(scheme_t s, scheme_t v)
{
    if (scheme_pairp(s) == SCHEME_FALSE)
        error("set-car!: attempt to set car of non-pair");
    GET_CAR(GET_PTR(s)) = (scheme_t)v;
    return SCHEME_UNSPEC;
}

scheme_t scheme_set_cdrx(scheme_t s, scheme_t v)
{
    if (scheme_pairp(s) == SCHEME_FALSE)
        error("set-cdr!: attempt to set cdr of non-pair");
    GET_CDR(GET_PTR(s)) = (scheme_t)v;
    return SCHEME_UNSPEC;
}
