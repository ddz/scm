/* $Id$ */

#ifndef SCHEME_H
#define SCHEME_H

#include "_types.h"

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
 * Scheme Procedures (organized by R5RS sections)
 */

/* 6.3.2. Pairs and lists */

#define scheme_pairp(s)       (IS_PAIRPTR(s))
extern scheme_t scheme_cons(scheme_t, scheme_t);
#define scheme_car(s)         (GET_CAR(GET_PTR(s)))
#define scheme_cdr(s)         (GET_CDR(GET_PTR(s)))
#define scheme_set_carx(s, v) (GET_CAR(GET_PTR(s)) = (scheme_t)v)
#define scheme_set_cdrx(s, v) (GET_CDR(GET_PTR(s)) = (scheme_t)v)


/* 6.6.3 Output */
extern scheme_t scheme_write_1(scheme_t);

/*
 * Other cruft
 */

#include <setjmp.h>

extern jmp_buf top_level;
extern char* error_msg;
#define error(msg) do { error_msg = msg; longjmp(top_level, 1); } while (0)

#endif
