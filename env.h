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

typedef struct {
    scm_hash_t  bindings;
    env_frame_t environment;
} env_frame_t;

#endif
