/* $Id$ */
/*
 * Scheme environments
 */

#include <stdlib.h>
#include "scheme.h"
#include "env.h"

extern size_t hash_symbol(const void*);

int eqp(const void* a, const void* b)
{
    return !scheme_eqp((scheme_t)a, (scheme_t)b);
}

env_frame_t* make_environment(env_frame_t* parent)
{
    env_frame_t* e = malloc(sizeof(env_frame_t));
    e->env = parent;
    e->bindings = malloc(sizeof(map_t));

    map_init(e->bindings, hash_symbol, eqp);

    return e;
}

int env_bind(env_frame_t* env, scheme_t var, scheme_t val)
{
    return map_put(env->bindings, (void*)var, (void*)val);
}

int env_lookup(env_frame_t* env, scheme_t var, scheme_t* val)
{
    void* v;

    v = map_get(env->bindings, (const void*)var);

    if (v == NULL)
        return 0;
    else
        *val = (scheme_t)v;
    return 1;
}
