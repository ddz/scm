/* $Id$ */
/*
 * 6.2.5 Numerical operations
 */

#include <stdlib.h>
#ifdef GMP
#include <gmp.h>
#endif
#include "scheme.h"

scheme_t make_bignum(char* str, int radix)
{
#ifdef GMP
    scheme_t s;
    mpz_t* bignum = malloc(sizeof(mpz_t));

    if (mpz_init_set_str(*bignum, str, radix) == -1) {
	mpz_clear(*bignum);
	free(bignum);
	error("mpz_init_set_str failed");
    }
    
    s = MAKE_CELL();

    GET_CAR(GET_PTR(s)) = (BIGNUM_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)bignum;

    return s;
#else
    error("bignums not supported");
#endif
}

scheme_t make_ratnum(char* num, char* den, int radix)
{
#ifdef GMP
    scheme_t s;
    mpq_t* ratnum = malloc(sizeof(mpq_t));

    mpq_init(*ratnum);

    if ((mpz_set_str(mpq_numref(*ratnum), num, radix) == -1) ||
	(mpz_set_str(mpq_denref(*ratnum), den, radix) == -1)) {
	mpq_clear(*ratnum);
	free(ratnum);
	error("mpz_set_str failed");
    }

    s = MAKE_CELL();
    GET_CAR(GET_PTR(s)) = (RATNUM_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)ratnum;

    return s;
#else
    error("ratnums not supported");
#endif
}

scheme_t scheme_plus(scheme_t args)
{
    int a = 0;
    scheme_t s = args;
    
    while (s != SCHEME_NIL) {
	scheme_t n = scheme_car(s);
	a += GET_FIXNUM(n);
	s = scheme_cdr(s);
    }
    
    return MAKE_FIXNUM(a);
}

scheme_t scheme_times(scheme_t args)
{
    int a = 1;
    scheme_t s = args;
    
    while (s != SCHEME_NIL) {
	scheme_t n = scheme_car(s);
	a *= GET_FIXNUM(n);
	s = scheme_cdr(s);
    }
    
    return MAKE_FIXNUM(a);
}

scheme_t scheme_minus(scheme_t a, scheme_t b)
{
    return MAKE_FIXNUM(GET_FIXNUM(a) - GET_FIXNUM(b));
}

scheme_t scheme_equals(scheme_t a, scheme_t b)
{
    if (GET_FIXNUM(a) == GET_FIXNUM(b))
	return SCHEME_TRUE;
    return SCHEME_FALSE;
}
