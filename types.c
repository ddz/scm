/* $Id$ */
/*
 * Miscellaneous type functions and constructors
 */

#include <stdlib.h>
#include "scheme.h"

scheme_t make_vector(scheme_t* vector, size_t elems)
{
    scheme_t s = MAKE_CELL();
    scheme_set_carx(s, (elems << 5) | (VECTOR_T << 3) | 6);
    scheme_set_cdrx(s, vector);
    return s;
}

scheme_t make_string(char* name, size_t len)
{
    scheme_t s = MAKE_CELL();
    scheme_set_carx(s, (len << 5) | (STRING_T << 3) | 6);
    scheme_set_cdrx(s, name);
    return s;
}
