/* $Id$ */
/*
 * Scheme procedures
 */

#ifndef PROCEDURES_H
#define PROCEDURES_H

#include "scheme.h"
#include "env.h"

struct procedure {
    enum {PRIMATIVE, COMPOUND} type;
    union {
	struct {
	    scheme_t (*f)(scheme_t);
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

extern scheme_t make_procedure(env_frame_t*, scheme_t, scheme_t);

#endif
