/* $Id$ */
/*
 * Standard Scheme procedures
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
