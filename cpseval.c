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

/*
 * A dirty hack for now while writing the CPS interpreter
 */

enum cont_type {
    HALT, TEST, VARASSIGN, DEFINITION, PRIM_ARGS, BEGIN,
    EVAL_RATOR, EVAL_RANDS, EVAL_FIRST, EVAL_REST
};

typedef struct _continuation {
    env_frame_t*          envt;
    struct _continuation* cont;

    enum cont_type type;
    
    union {
        struct {
            scheme_t true_expr;
            scheme_t false_expr;
        } test;
        
        struct {
            scheme_t var;
        } assignment;
        
        struct {
            scheme_t prim;
        } prim_args;

        struct {
            scheme_t exprs;
        } eval_begin;
            
        struct {
            scheme_t rands;
        } eval_rator;

        struct {
            scheme_t proc;
        } eval_rands;

        struct {
            scheme_t exprs;
        } eval_first;

        struct {
            scheme_t first_value;
        } eval_rest;
    } data;
} continuation_t;

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

scheme_t scheme_eval(scheme_t sexpr, env_frame_t* e)
{
    /*
     * Global "registers"
     */
    scheme_t        expr = sexpr;
    env_frame_t*    env = e;
    continuation_t* cont = make_continuation(HALT, NULL, NULL);
    scheme_t        rands;
    scheme_t        val;
    scheme_t        proc;
    scheme_t        args;
    
    
 EVAL_EXPRESSION:
    if (IS_SYMBOL(expr)) {
        if (env_lookup(env, expr, &val))
            goto APPLY_CONT;
        error("reference to undefined identifier");
    }
    else if (scheme_pairp(expr)) {
        scheme_t rator, rands;

        rator = scheme_car(expr);
        rands = scheme_cdr(expr);

        if (IS_SYMBOL(rator) &&
	    env_lookup(env, rator, &rator) &&
	    IS_SYNT(rator)) {
	    
            switch (rator) {
            case SCHEME_BEGIN: {
                cont = make_continuation(BEGIN, NULL, cont);
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

            default:
                error("Syntactic form not yet implemented");
            }
        }
        else {
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
	free_cont(cont);
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
	
        if (env_lookup(cont->envt,
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
        
    case PRIM_ARGS: {
	continuation_t* old_cont = cont;

        // val = apply_primative(cont->data.prim_args.prim, val);
        error("Primatives don't exist yet");

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
        cont = make_continuation(EVAL_REST, NULL, cont->cont);
        cont->data.eval_rest.first_value = val;

	free_cont(old_cont);
	
        goto EVAL_RANDS;
    }

    case EVAL_REST: {
	continuation_t* old_cont = cont;
        val = scheme_cons(cont->data.eval_rest.first_value, val);
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
            /*
	    continuation_t* old_cont = cont;
	    cont = cont->cont;
	    free(old_cont);
            */

	    val = (*p->data.primative.f)(args);

            goto APPLY_CONT;
	}
	else {
	    scheme_t vars = p->data.compound.formals;
	    scheme_t vals = args;
        
	    /*
	     * Extend lexical environment with formal parameters
	     */
	    env = p->data.compound.env;
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

	    // Evaluate body as implicit begin
	    cont = make_continuation(BEGIN, NULL, cont);
	    cont->data.eval_begin.exprs = p->data.compound.body;
	
	    goto APPLY_CONT;
	}
    }
}