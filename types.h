/* $Id$ */
/*
 * Scheme Type definitions and macros
 */

#ifndef TYPES_H
#define TYPES_H

#include "_types.h"

/*
 * Scheme constants
 */
#define SCHEME_TRUE BOOL_TRUE
#define SCHEME_FALSE BOOL_FALSE
#define SCHEME_NIL UNIQ_NIL
#define SCHEME_EOF UNIQ_EOF
#define SCHEME_UNDEF UNIQ_UNDEF
#define SCHEME_UNSPEC UNIQ_UNSPEC

/*
 * Macro implementations of scheme built-in procedures following the
 * naming convention SCHEME_<proc>, whith 'P' replacing '?', 'X'
 * replacing '!', and '2' replacing '->'.
 */

#define SCHEME_INTEGERP(s) (IS_FIXNUM(s) | IS_BIGNUM(s))
#define SCHEME_RATIONALP(s) (IS_RATNUM(s) | SCHEME_INTEGERP(s))
#define SCHEME_REALP(s) (IS_FLONUM(s) | IS_REALNUM(s) | SCHEME_RATIONALP(s))
#define SCHEME_COMPLEXP(s) (IS_RECTNUM(s) | IS_COMPNUM(s) | SCHEME_REALP(s))
#define SCHEME_NUMBERP(s) SCHEME_COMPLEXP(s)

#define SCHEME_EXACTP(s) (SCHEME_RATIONALP(s) | IS_REALNUM(s) | IS_RECTNUM(s))
#define SCHEME_INEXACTP(s) (IS_FLONUM(s) | IS_COMPNUM(s))

#define SCHEME_CAR(s)         (GET_CAR(GET_PTR(s)))
#define SCHEME_CDR(s)         (GET_CDR(GET_PTR(s)))
#define SCHEME_SET_CARX(s, v) (GET_CAR(GET_PTR(s)) = (scheme_t)v)
#define SCHEME_SET_CDRX(s, v) (GET_CDR(GET_PTR(s)) = (scheme_t)v)

#define SCHEME_CONS(car, cdr) (scheme_cons(car, cdr))
#define MAKE_SYMBOL(str, size) (make_symbol(str, size))
#define MAKE_STRING(str, size) (make_string(str, size))

extern scheme_t scheme_cons(scheme_t, scheme_t);
extern scheme_t make_symbol(char*, size_t);
extern scheme_t make_string(char*, size_t);


#endif
