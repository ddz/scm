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

    p->env = env;
    p->formals = formals;
    p->body = body;
    
    scheme_set_carx(s, (PROCEDURE_T << 3) | 6);
    scheme_set_cdrx(s, p);
}
