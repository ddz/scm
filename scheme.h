/* $Id$ */

#ifndef SCHEME_H
#define SCHEME_H

#include "_types.h"
#include "procedures.h"

/*
 * All Scheme constants and procedures following are prefixed with
 * SCHEME_ (constant) or scheme_ (procedures).  Special characters (?,
 * !, ->) in procedure names are mapped to C-friendly equivalents (P,
 * X, 2, respectively).  All procedure names are lower case whether
 * they are implemented as macros or functions.  If a procedure may
 * take a variable number of arguments (and the number is not
 * determinable by the first argument, as C varargs requires), the
 * procedure name is suffixed with _<number of args>.
 */

/*
 * Scheme constants (booleans, unique values, and syntactic keywords)
 */
#define SCHEME_TRUE             BOOL_TRUE
#define SCHEME_FALSE            BOOL_FALSE
#define SCHEME_NIL              UNIQ_NIL
#define SCHEME_EOF              UNIQ_EOF
#define SCHEME_UNDEF            UNIQ_UNDEF
#define SCHEME_UNSPEC           UNIQ_UNSPEC

/*
 * Scheme special forms
 * Primitive special forms: quote, lambda, if, set!, let-syntax,
 *                          letrec-syntax, quasiquote.
 * define, define-syntax ?
 */

#define SCHEME_QUOTE            SYNT_QUOTE
#define SCHEME_LAMBDA           SYNT_LAMBDA
#define SCHEME_IF               SYNT_IF              
#define SCHEME_SETX             SYNT_SETX            
#define SCHEME_BEGIN            SYNT_BEGIN           
#define SCHEME_COND             SYNT_COND            
#define SCHEME_AND              SYNT_AND             
#define SCHEME_OR               SYNT_OR              
#define SCHEME_CASE             SYNT_CASE            
#define SCHEME_LET              SYNT_LET             
#define SCHEME_LETS             SYNT_LETS            
#define SCHEME_LETREC           SYNT_LETREC          
#define SCHEME_DO               SYNT_DO              
#define SCHEME_DELAY            SYNT_DELAY           
#define SCHEME_QUASIQUOTE       SYNT_QUASIQUOTE      
#define SCHEME_ELSE             SYNT_ELSE            
#define SCHEME_EQGT             SYNT_EQGT            
#define SCHEME_DEFINE           SYNT_DEFINE          
#define SCHEME_UNQUOTE          SYNT_UNQUOTE         
#define SCHEME_UNQUOTE_SPLICING SYNT_UNQUOTE_SPLICING


/*
 * Scheme primitive procedures (organized by R5RS sections)
 */

/* 6.1. Equivalence predicates */
// eqv?
#define scheme_eqp(a, b)      (a == b)

/* 6.2.5. Numerical operations */
#define scheme_numberp(s)     (IS_FIXNUM(s))
// complex?
// real?
// rational?
// integer?
// exact?
// inexact?
// =
// <
// >
// <=
// >=
// +
// *
// -
// /
// quotient
// remainder
// modulo
// numerator
// denominator
// floor
// ceiling
// truncate
// round
// exp
// log
// sin
// cos
// tan
// asin
// acos
// atan
// sqrt
// expt
// make-rectangular
// make-polar
// real-part
// imag-part
// magnitude
// angle
// exact->inexact
// inexact->exact


/* 6.2.6. Numerical input and output */
// number->string
extern scheme_t scheme_string2number_1(scheme_t);
extern scheme_t scheme_string2number_2(scheme_t, scheme_t);

/* 6.3.1. Booleans */
// boolean?  (??? R5RS says "library procedure", how?)

/* 6.3.2. Pairs and lists */

extern scheme_t scheme_pairp(scheme_t);
extern scheme_t scheme_cons(scheme_t, scheme_t);
extern scheme_t scheme_car(scheme_t);
extern scheme_t scheme_cdr(scheme_t);
extern scheme_t scheme_set_carx(scheme_t, scheme_t);
extern scheme_t scheme_set_cdrx(scheme_t, scheme_t);

/* 6.3.3. Symbols */
// symbol?
// symbol->string
// string->symbol

/* 6.3.4. Characters */
// char?
// char=?
// char<?
// char>?
// char<=?
// char>=?
// char->integer
// integer->char

/* 6.3.5 Strings */
// string?
// make-string
// string-length
// string-ref
// string-set!

/* 6.3.6. Vectors */
// vector?
// make-vector
// vector-length
// vector-ref
// vector-set!

/* 6.4. Control features */
// procedure?
// apply
// call-with-current-continuation
// values
// call-with-values
// dynamic-wind


/* 6.5. Eval */
// eval
// scheme-report-environment
// null-environment
// interaction-environment (optional)

/* 6.6.1. Ports */
// input-port?
// output-port?
// current-input-port
// current-output-port
// with-input-from-file (optional)
// with-output-to-file (optional)
// open-input-file
// open-output-file
// close-input-port
// close-output-port

/* 6.6.2. Input */
// XXX: Write read as library procedure
// read-char
// peek-char
// eof-object?
// char-ready?

/* 6.6.3 Output */
extern scheme_t scheme_write_1(scheme_t); // XXX: Write as lib proc
// write-char

/* 6.6.4. System interface */
// load (optional)
// transcript-on (optional)
// transcript-off (optional)

/*
 * Other cruft
 */

#include <setjmp.h>

extern jmp_buf top_level;
extern char* error_msg;
#define error(msg) do { error_msg = msg; longjmp(top_level, 1); } while (0)

extern scheme_t scheme_eval(scheme_t, env_frame_t*);

#endif
