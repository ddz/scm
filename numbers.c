/* $Id$ */
/*
 * 6.2.5 Numerical operations
 */

#include <stdlib.h>
#include <gmp.h>
#include "scheme.h"

scheme_t make_bignum(char* str, int radix)
{
    scheme_t s;
    mpz_t* bignum = malloc(sizeof(mpz_t));

    if (mpz_init_set_str(*bignum, str, radix) == -1) {
	mpz_clear(*bignum);
	free(bignum);
	error("mpz_init_set_str failed");
    }
    
    s = MAKE_CELL();
    scheme_set_carx(s, (BIGNUM_T << 3) | 6);
    scheme_set_cdrx(s, bignum);
    return s;
}

scheme_t make_ratnum(char* num, char* den, int radix)
{
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
    scheme_set_carx(s, (RATNUM_T << 3) | 6);
    scheme_set_cdrx(s, ratnum);
    return s;
}
