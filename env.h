/* $Id$ */
/*
 * Scheme Environments
 */

#ifndef ENV_H
#define ENV_H

#include "scheme.h"
#include "map.h"

/*
 * A Scheme environment is a sequence of frames, each containing a
 * table of variable->value bindings and a pointer to its enclosing
 * environment (unless it is the global environment).
 */

typedef struct _env_frame {
    struct _env_frame* env;       /* Enclosing (parent) environment */
    map_t*             bindings;
} env_frame_t;

extern env_frame_t* make_environment(env_frame_t*);
extern int          env_bind(env_frame_t*, scheme_t, scheme_t);
extern int          env_lookup(env_frame_t*, scheme_t, scheme_t*);

#endif
