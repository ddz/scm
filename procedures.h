/* $Id$ */
/*
 * Scheme procedures
 */

#ifndef PROCEDURES_H
#define PROCEDURES_H

#include "scheme.h"
#include "env.h"

typedef scheme_t (*primative_t)();

struct procedure {
    enum {PRIMATIVE, COMPOUND} type;
    union {
	struct {
	    primative_t f;
	    uint8_t     nargs_req;
	    uint8_t     nargs_opt;
	    uint8_t     rest;
	} primative;
	struct {
	    env_frame_t* env;        /* Reference to lexical environment */
	    scheme_t     formals;    /* List of formal parameters */
	    scheme_t     body;       /* List of body expressions  */
	} compound;
    } data;
};

#define MAKE_PRIMATIVE(f) (make_primative(f));
#define MAKE_PROCEDURE(e, f, b) (make_procedure(e, a, b))
#define GET_PROCEDURE(s)   ((struct procedure*)GET_CDR(GET_PTR(s)))

/*
 * Make a new compound procedure in the given lexical environment with
 * the passed list of formals and list of body expressions.
 */
extern scheme_t make_procedure(env_frame_t*, scheme_t, scheme_t);

/*
 * Make a new primative procedure given a function pointer, the number
 * of required arguments, optional arguments, and whether or not to
 * pass a 'rest' argument.
 */
extern scheme_t make_primative(primative_t proc,
			       uint8_t nargs_req,
			       uint8_t nargs_opt,
			       uint8_t rest);

/*
 * Apply a primative procedure to the list of arguments.
 */
extern scheme_t apply_primative(scheme_t rator, scheme_t rands);

#endif
