/* $Id$ */
/*
 * Scheme Environments
 */

#ifndef ENV_H
#define ENV_H

/*
 * A Scheme environment is a sequence of frames, each containing a
 * table of variable->value bindings and a pointer to its enclosing
 * environment (unless it is the global environment).
 */

typedef struct _env_frame {
    map_t*             bindings;
    struct _env_frame* environment;
} env_frame_t;

#endif
