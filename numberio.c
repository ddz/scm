/* $Id$ */
/*
 * 6.2.6 Numerical input and output
 */

#include <stdlib.h>
#include <limits.h>
#include "scheme.h"

static scheme_t parse_complex(char*, int);
static scheme_t parse_real(char*, int, char**);
static scheme_t parse_uinteger(char*, int, char**);
static scheme_t parse_decimal(char*);

scheme_t scheme_string2number_1(scheme_t str)
{
    /* Default to radix 10 */
    return scheme_string2number_2(str, MAKE_FIXNUM(10));
}

scheme_t scheme_string2number_2(scheme_t s, scheme_t b)
{
    int i;
    char* str = GET_STRING(s);
    int radix = GET_FIXNUM(b);
    int exactness = 1;            /* Default to exact numbers */
    
    if (!IS_STRING(s) || !IS_FIXNUM(b) ||
	(radix != 2 && radix != 8 && radix != 10 && radix != 16))
	return SCHEME_FALSE;

    /*
     * Parse prefix to determine radix and exactness (if specified)
     */
    for (i = 0; i < 2 && *str == '#'; i++) {
	switch(*++str) {
	case 'e':
	    exactness = 1;
	    break;
	case 'i':
	    exactness = 0;
	    break;
	case 'b':
	    radix = 2;
	    break;
	case 'o':
	    radix = 8;
	    break;
	case 'd':
	    radix = 10;
	    break;
	case 'x':
	    radix = 16;
	    break;

	default:
	    return SCHEME_FALSE;
	}
    }

    // XXX: Do something about explicit exactness (convert)
    return parse_complex(str + i, radix);
}

scheme_t parse_complex(char* str, int radix)
{
    char* endptr = NULL;
    scheme_t real, imag;
    
    real = parse_real(str, radix, &endptr);
    if (*endptr != NULL) {
	switch (*endptr) {
	case '@':
	    imag = parse_real(endptr + 1, radix, NULL);
	    printf("Make polar\n");
	    return imag;

	case '+':
	case '-':
	    imag = parse_real(endptr, radix, &endptr);
	    if (*endptr == 'i' || *endptr == 'I') {
		printf("Make imaginary\n");
		return imag;
	    }
	    else if (*endptr == '+') {
		// +i
		return SCHEME_FALSE;
	    }
	    else if (*endptr == '-') {
		// -i
		return SCHEME_FALSE;
	    }
	    return SCHEME_FALSE;

	case 'i':
	case 'I':
	    printf("Make imaginary\n");
	    return imag;

	default:
	    return SCHEME_FALSE;
	}
    }
    
    return real;
}

scheme_t parse_real(char* str, int radix, char** endptr)
{
    scheme_t num, denom;
    
    if (strchr(str, '.') || strchr(str, '#')) {
	if (radix == 10)
	    return parse_decimal(str);
	else
	    return SCHEME_FALSE;
    }

    num = parse_uinteger(str, radix, endptr);
    
    if (endptr && **endptr == '/') {
	denom = parse_uinteger(*endptr + 1, radix, NULL);
	printf("make ratio\n");
	return denom;
    }
    else
	return num;
}

scheme_t parse_uinteger(char* str, int radix, char** endptr)
{
    long fixnum;
    
    fixnum = strtol(str, endptr, radix);
    if (fixnum == LONG_MIN || fixnum == LONG_MAX)
	// XXX: Make bignum
	return SCHEME_FALSE;
    else {
	scheme_t n = MAKE_FIXNUM(fixnum);
	if (GET_FIXNUM(n) != fixnum)
	    // XXX: Make bignum
	    return SCHEME_FALSE;
	return n;
    }
}

scheme_t parse_decimal(char* str)
{
    return SCHEME_FALSE;
}
