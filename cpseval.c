/* $Id$ */
/*
 * Continuation Passing Style evaluator modeled after the imperative
 * interpreter written in Scheme in Essentials of Programming
 * Languages (Friedman, Wand, Haynes).
 *
 * This makes the interpreter fully tail-recursive, utilizing tail
 * calls wherever possible, not just when tail recursive functions
 * call themselves.
 */

#include <stdlib.h>
#include "scheme.h"
#include "map.h"

/*
 * A dirty hack for now while writing the CPS interpreter
 */

/*
 * Global "registers"
 */
scheme_t        expr = SCHEME_NIL;
env_frame_t*    env = NULL;
continuation_t* cont = NULL;
scheme_t        rator = SCHEME_NIL;
scheme_t        rands = SCHEME_NIL;
scheme_t        val = SCHEME_NIL;
scheme_t        proc = SCHEME_NIL;
scheme_t        args = SCHEME_NIL;

static char errbuf[2048];

#define ENV_CACHE_SIZE 10;
struct env_cache_entry {
    env_frame_t*  env;
    map_entry_t** me;
};
static struct env_cache_entry env_cache[5];

continuation_t* make_continuation(enum cont_type t,
				  env_frame_t* e,
				  continuation_t* c)
{
    continuation_t* cont = malloc(sizeof(continuation_t));
    cont->type = t;
    cont->envt = e;
    cont->cont = c;
    
    return cont;
}

void free_cont(continuation_t* c)
{
    free(c);
}

scheme_t mapcar(scheme_t ls)
{
    scheme_t s;
    if (ls == SCHEME_NIL)
	return SCHEME_NIL;
    s = scheme_car(ls);
    return scheme_cons(scheme_car(s), mapcar(scheme_cdr(ls)));
}

scheme_t mapcadr(scheme_t ls)
{
    scheme_t s;
    if (ls == SCHEME_NIL)
	return SCHEME_NIL;
    s = scheme_car(ls);
    return scheme_cons(scheme_car(scheme_cdr(s)),
		       mapcadr(scheme_cdr(ls)));
}

scheme_t make_let(scheme_t vars, scheme_t vals, scheme_t body)
{
    return scheme_cons(scheme_cons(MAKE_SYMBOL("lambda", 6),
				   scheme_cons(vars, body)), vals);
}

int apply_env(env_frame_t* env, scheme_t var, scheme_t* valptr)
{
    env_frame_t* e = env;
    size_t n;
    map_entry_t** me = NULL;

    n = (unsigned int)var % ENV_CACHE_SIZE;

    /*
     * Check the cache first
     */
    if (env_cache[n].env == e &&
        (scheme_t)(*env_cache[n].me)->key == var) {
        if (valptr)
            *valptr = (scheme_t)(*env_cache[n].me)->data;
        return 1;
    }
    else {
        while (e != NULL) {
            me = map_lookup(e->bindings, (void*)var);
            if (me == NULL || *me == NULL) {
                e = e->env;
                continue;
            }

            env_cache[n].env = env;
            env_cache[n].me = me;

            if (valptr)
                *valptr = (scheme_t)(*me)->data;
            return 1;
        }

        return 0;
    }
}

scheme_t scheme_apply(scheme_t proc, scheme_t args)
{
    struct procedure* p = GET_PROCEDURE(proc);
    
    if (p->type == PRIMATIVE)
	return apply_primative(proc, args);
    else
	error("Can't apply non-primative procedure yet");
}

scheme_t scheme_eval(scheme_t sexpr, env_frame_t* e)
{
    expr = sexpr;
    env = e;
    cont = make_continuation(HALT, env, NULL);
    
 EVAL_EXPRESSION:
    if (IS_SYMBOL(expr)) {
        if (apply_env(env, expr, &val))
            goto APPLY_CONT;
        snprintf(errbuf, 2000,
                 "reference to undefined identifier: %s",
                 GET_SYMBOL_NAME(expr));
        error(errbuf);
    }
    else if (scheme_pairp(expr) == SCHEME_TRUE) {
        scheme_t r;
        r = rator = scheme_car(expr);
        rands = scheme_cdr(expr);

        if (IS_SYMBOL(rator) &&
	    apply_env(env, rator, &rator) &&
	    IS_IMMVAL(rator) && IS_SYNT(rator)) {
	    
            switch (rator) {
            case SCHEME_BEGIN: {
                cont = make_continuation(BEGIN, env, cont);
                cont->data.eval_begin.exprs = rands;
            }
                
            case SCHEME_LAMBDA: {
                scheme_t formals, body;

                formals = scheme_car(rands);
                body = scheme_cdr(rands);
                
                val = make_procedure(env, formals, body);
                goto APPLY_CONT;
            }
                
            case SCHEME_QUOTE: {
                val = scheme_car(rands);
                if (scheme_cdr(rands) != SCHEME_NIL)
                    error("quote: too many arguments");
                goto APPLY_CONT;
            }

            case SCHEME_IF: {
                scheme_t test, clauses, true_expr, false_expr;
                
                test = scheme_car(rands);
                clauses = scheme_cdr(rands);

                if (clauses == SCHEME_NIL)
                    error("if: too few arguments");

                true_expr = scheme_car(clauses);

                if (scheme_cdr(clauses) == SCHEME_NIL)
                    false_expr = SCHEME_UNSPEC;
                else
                    false_expr = scheme_car(scheme_cdr(clauses));

                expr = test;
                cont = make_continuation(TEST, env, cont);
                cont->data.test.true_expr = true_expr;
                cont->data.test.false_expr = false_expr;

                goto EVAL_EXPRESSION;
            }

            case SCHEME_DEFINE: {
                if (rands == SCHEME_NIL)
                    error("define: too few expressions");

                cont = make_continuation(DEFINITION, env, cont);
                cont->data.assignment.var = scheme_car(rands);

                if (scheme_cdr(rands) == SCHEME_NIL)
                    error("define: too few expressions");

                expr = scheme_car(scheme_cdr(rands));

                goto EVAL_EXPRESSION;
            }
                
            case SCHEME_SETX: {
                if (rands == SCHEME_NIL)
                    error("set!: too few expressions");

                cont = make_continuation(VARASSIGN, env, cont);
                cont->data.assignment.var = scheme_car(rands);

                if (scheme_cdr(rands) == SCHEME_NIL)
                    error("set!: too few expressions");

                expr = scheme_car(scheme_cdr(rands));

                goto EVAL_EXPRESSION;
            }

	    case SCHEME_LET: {
		scheme_t vars = mapcar(scheme_car(rands));
		scheme_t vals = mapcadr(scheme_car(rands));
		scheme_t body = scheme_cdr(rands);

		expr = make_let(vars, vals, body);
		
		goto EVAL_EXPRESSION;
	    }

            default:
                error("Syntactic form not yet implemented");
            }
        }
        else if (rator == SCHEME_UNDEF) {
            snprintf(errbuf, 2000,
                     "reference to undefined identifier: %s",
                     GET_SYMBOL_NAME(r));
            error(errbuf);
        }
        else {
            /*
             * Procedure application
             */
            expr = rator;
            cont = make_continuation(EVAL_RATOR, env, cont);
            cont->data.eval_rator.rands = rands;

            goto EVAL_EXPRESSION;
        }
    }
    else {
        val = expr;
        goto APPLY_CONT;
    }

 APPLY_CONT:
    switch (cont->type) {
    case HALT:
	env = cont->envt;
	free_cont(cont);
	cont = NULL;
        return val;

    case TEST: {
	continuation_t* old_cont = cont;
	
        if (val != SCHEME_FALSE)
            expr = cont->data.test.true_expr;
        else
            expr = cont->data.test.false_expr;
        env = cont->envt;
        cont = cont->cont;

	free_cont(old_cont);
	
        goto EVAL_EXPRESSION;
    }
	
    case VARASSIGN: {
	continuation_t* old_cont = cont;
	
        if (apply_env(cont->envt,
                      cont->data.assignment.var, NULL)) {
            env_bind(cont->envt,
                     cont->data.assignment.var, val);
            val = SCHEME_UNSPEC;
            cont = cont->cont;

	    free_cont(old_cont);
	    
            goto APPLY_CONT;
        }
        else
            error("set!: variable not bound");
    }
        
    case DEFINITION: {
	continuation_t* old_cont = cont;
	
        env_bind(cont->envt,
                 cont->data.assignment.var, val);
        val = SCHEME_UNSPEC;
        cont = cont->cont;
	free_cont(old_cont);
	
        goto APPLY_CONT;
    }
        
    case BEGIN: {
	continuation_t* old_cont = cont;
	
        expr = scheme_car(cont->data.eval_begin.exprs);
        cont->data.eval_begin.exprs =
            scheme_cdr(cont->data.eval_begin.exprs);
	
        if (cont->data.eval_begin.exprs == SCHEME_NIL) {
            cont = cont->cont;
	    // env = cont->envt;
	    free_cont(old_cont);
	}
        
        goto EVAL_EXPRESSION;
    }
    
    case EVAL_RATOR: {
	continuation_t* old_cont = cont;
        
        rands = cont->data.eval_rator.rands;
        env = cont->envt;
        cont = make_continuation(EVAL_RANDS, NULL, cont->cont);
        cont->data.eval_rands.proc = val;
	
	free_cont(old_cont);
	
        goto EVAL_RANDS;
    }

    case EVAL_RANDS:
        if (IS_PROCEDURE(cont->data.eval_rands.proc)) {
	    continuation_t* old_cont = cont;
            proc = cont->data.eval_rands.proc;
            args = val;
            cont = cont->cont;

	    free_cont(old_cont);
	    
            goto APPLY_PROCVAL;
        }
        else
            error("Attempt to apply non-procedure");

    case EVAL_FIRST: {
	continuation_t* old_cont = cont;
        
        rands = scheme_cdr(cont->data.eval_first.exprs);
        env = cont->envt;
        cont = make_continuation(EVAL_REST, env, cont->cont);
        cont->data.eval_rest.first_value = val;

	free_cont(old_cont);
	
        goto EVAL_RANDS;
    }

    case EVAL_REST: {
	continuation_t* old_cont = cont;
        scheme_t v = scheme_cons(cont->data.eval_rest.first_value, val);
        val = v;
        cont = cont->cont;

	free_cont(old_cont);
	
        goto APPLY_CONT;
    }
    }

 EVAL_RANDS:
    if (rands == SCHEME_NIL) {
        val = SCHEME_NIL;
        goto APPLY_CONT;
    }
    else {
        expr = scheme_car(rands);
        cont = make_continuation(EVAL_FIRST, env, cont);
        cont->data.eval_first.exprs = rands;

        goto EVAL_EXPRESSION;
    }
    
 APPLY_PROCVAL: {
        struct procedure* p = GET_PROCEDURE(proc);

	if (p->type == PRIMATIVE) {
	    val = apply_primative(proc, args);

            goto APPLY_CONT;
	}
	else {
	    scheme_t vars = p->data.compound.formals;
	    scheme_t vals = args;
        
	    /*
	     * Extend lexical environment with formal parameters
	     */
	    env = make_environment(p->data.compound.env);
	    while (vars != SCHEME_NIL &&
		   vals != SCHEME_NIL &&
		   IS_PAIRPTR(vars)) {
		scheme_t var, val;
		
		var = scheme_car(vars);
		val = scheme_car(vals);
		
		env_bind(env, var, val);
		
		vars = scheme_cdr(vars);
		vals = scheme_cdr(vals);
	    }
	    if (IS_SYMBOL(vars))
		env_bind(env, vars, vals);
            else if (vars != SCHEME_NIL)
		error("Not enough args to proc application");
	    
	    // Evaluate body as implicit begin
	    cont = make_continuation(BEGIN, NULL, cont);
	    cont->data.eval_begin.exprs = p->data.compound.body;
	
	    goto APPLY_CONT;
	}
    }
}
