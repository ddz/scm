/* $Id$ */
/*
 * Scheme procedures
 */

#include <stdlib.h>
#include "scheme.h"

scheme_t make_primative(scheme_t (*f)(scheme_t))
{
    scheme_t s = MAKE_CELL();
    struct procedure* p = malloc(sizeof(struct procedure));

    p->type = PRIMATIVE;
    p->data.primative.f = f;
    
    scheme_set_carx(s, (PROCEDURE_T << 3) | 6);
    scheme_set_cdrx(s, p);

    return s;
}

scheme_t make_procedure(env_frame_t* env, scheme_t formals, scheme_t body)
{
    scheme_t s = MAKE_CELL();
    struct procedure* p = malloc(sizeof(struct procedure));

    p->type = COMPOUND;
    p->data.compound.env = env;
    p->data.compound.formals = formals;
    p->data.compound.body = body;
    
    scheme_set_carx(s, (PROCEDURE_T << 3) | 6);
    scheme_set_cdrx(s, p);

    return s;
}
