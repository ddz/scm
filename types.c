/* $Id$ */
/*
 * Miscellaneous type functions and constructors
 */

#include <stdio.h>
#include <stdlib.h>
#include "scheme.h"

scheme_t make_vector(scheme_t* vector, size_t elems)
{
    scheme_t s = MAKE_CELL();
    GET_CAR(GET_PTR(s)) = (elems << 6) | (VECTOR_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)vector;
    return s;
}

scheme_t make_string(char* name, size_t len)
{
    scheme_t s = MAKE_CELL();
    GET_CAR(GET_PTR(s)) = (len << 6) | (STRING_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)name;
    return s;
}
