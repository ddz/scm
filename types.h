/* $Id$ */
/*
 * Scheme Type definitions and macros
 */

#ifndef TYPES_H
#define TYPES_H

#include "_types.h"

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

#endif
