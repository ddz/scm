/* $Id$ */
/*
 * Scheme R5RS pairs and lists procedures
 */

#include <stdlib.h>
#include "scheme.h"

scheme_t scheme_cons(scheme_t car, scheme_t cdr)
{
    scheme_t s = MAKE_PAIR();
    scheme_set_carx(s, car);
    scheme_set_cdrx(s, cdr);
    return s;
}

scheme_t scheme_append(scheme_t ls1, scheme_t ls2)
{
    if (ls1 == SCHEME_NIL)
        return ls2;
    else if (ls2 == SCHEME_NIL)
        return ls1;

    return
        scheme_append(scheme_cons(ls1,
                                  scheme_cons(scheme_car(ls2),
                                              SCHEME_NIL)),
                      scheme_cdr(ls2));
}
