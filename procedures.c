/* $Id$ */
/*
 * Scheme procedures
 */

#include <stdlib.h>
#include "scheme.h"

scheme_t make_procedure(env_frame_t* env, scheme_t formals, scheme_t body)
{
    scheme_t s = MAKE_CELL();
    struct procedure* p = malloc(sizeof(struct procedure));

    p->type = COMPOUND;
    p->data.compound.env = env;
    p->data.compound.formals = formals;
    p->data.compound.body = body;
    
    GET_CAR(GET_PTR(s)) = (PROCEDURE_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)p;

    return s;
}

scheme_t make_primative(primative_t f, uint8_t req,
			uint8_t opt, uint8_t rest)
{
    scheme_t s = MAKE_CELL();
    struct procedure* p = malloc(sizeof(struct procedure));

    p->type = PRIMATIVE;
    p->data.primative.f = f;
    p->data.primative.nargs_req = req;
    p->data.primative.nargs_opt = opt;
    p->data.primative.rest = rest;
    
    GET_CAR(GET_PTR(s)) = (PROCEDURE_T << 3) | 6;
    GET_CDR(GET_PTR(s)) = (scheme_t)p;

    return s;
}

scheme_t apply_primative(scheme_t rator, scheme_t rands)
{
    struct procedure* p = GET_PROCEDURE(rator);
    int i, n = 0, nargs = p->data.primative.nargs_req +
        p->data.primative.nargs_opt + (p->data.primative.rest ? 1: 0);
    primative_t f = p->data.primative.f;
    scheme_t args[6];

    /*
     * Fill array of arguments to the function
     */
    for (i = 0; i < p->data.primative.nargs_req; i++, n++) {
        if (rands == SCHEME_NIL) {
            error("not enough arguments to primative precedure");
        }
        args[n] = scheme_car(rands);
        rands = scheme_cdr(rands);
    }

    for (i = 0; i < p->data.primative.nargs_opt &&
             rands != SCHEME_NIL; i++, n++) {
        args[n] = scheme_car(rands);
        rands = scheme_cdr(rands);
    }

    if (p->data.primative.rest)
        args[n++] = rands;
    else if (rands != SCHEME_NIL)
        error("Too many arguments to primative procedure");
    
    /*
     * Make the call
     */
    switch (n) {
    case 0: return (*f)();
    case 1: return (*f)(args[0]);
    case 2: return (*f)(args[0], args[1]);
    case 3: return (*f)(args[0], args[1], args[2]);
    case 4: return (*f)(args[0], args[1], args[2], args[3]);
    case 5: return (*f)(args[0], args[1], args[2], args[3], args[4]);
    case 6: return (*f)(args[0], args[1], args[2], args[3], args[4], args[5]);
    }
}
