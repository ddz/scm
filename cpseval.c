/* $Id$ */
/*
 * Continuation Passing Style evaluator modeled after the imperative
 * interpreter in Essentials of Programming Languages (Friedman, Wand,
 * Haynes).
 */

#include "scheme.h"

/*
 * A dirty hack for now while writing the CPS interpreter
 */
typedef struct _continuation {
    enum {
        HALT, TEST, VARASSIGN, DEFINITION, PRIM_ARGS, BEGIN,
        EVAL_RATOR, EVAL_RANDS, EVAL_FIRST, EVAL_REST
    } type;
    
    union {
        struct {
            scheme_t true_expr;
            scheme_t false_expr;
            env_frame_t* env;
            struct _continuation* cont;
        } test;
        
        struct {
            env_frame_t* env;
            scheme_t var;
            struct _continuation* cont;
        } assignment;
        
        struct {
            scheme_t prim;
            struct _continuation* cont;
        } prim_args;

        struct {
            scheme_t exprs;
            struct _continuation* cont;
        } eval_begin;
            
        struct {
            scheme_t rands;
            env_frame_t* env;
            struct _continuation* cont;
        } eval_rator;

        struct {
            scheme_t proc;
            struct _continuation* cont;
        } eval_rands;

        struct {
            scheme_t exprs;
            env_frame_t* env;
            struct _continuation* cont;
        } eval_first;

        struct {
            scheme_t first_value;
            struct _continuation* cont;
        } eval_rest;
    } data;
    
} continuation_t;

continuation_t halt_cont     = {HALT};
continuation_t test_cont     = {TEST};
continuation_t define_cont   = {DEFINITION};
continuation_t assign_cont   = {VARASSIGN};
continuation_t prim_arg_cont = {PRIM_ARGS};
continuation_t begin_cont    = {BEGIN};
continuation_t rator_cont    = {EVAL_RATOR};
continuation_t rands_cont    = {EVAL_RANDS};
continuation_t first_cont    = {EVAL_FIRST};
continuation_t rest_cont     = {EVAL_REST};

scheme_t scheme_eval(scheme_t sexpr, env_frame_t* e)
{
    /*
     * Global "registers"
     */
    scheme_t        expr = sexpr;
    env_frame_t*    env = e;
    continuation_t* cont = &halt_cont;
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

        rator = scheme_eval(scheme_car(expr), env);
        rands = scheme_cdr (expr);

        if (IS_SYNT(rator)) {
            switch (rator) {
            case SCHEME_BEGIN: {
                continuation_t* old_cont = cont;

                cont = &begin_cont;
                cont->data.eval_begin.exprs = rands;
                cont->data.eval_begin.cont = old_cont;
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
                continuation_t* old_cont = cont;
                
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
                cont = &test_cont;
                cont->data.test.true_expr = true_expr;
                cont->data.test.false_expr = false_expr;
                cont->data.test.env = env;
                cont->data.test.cont = old_cont;

                goto EVAL_EXPRESSION;
            }

            case SCHEME_DEFINE: {
                continuation_t* old_cont = cont;
                
                if (rands == SCHEME_NIL)
                    error("define: too few expressions");

                cont = &define_cont;
                cont->data.assignment.env = env;
                cont->data.assignment.var = scheme_car(rands);
                cont->data.assignment.cont = old_cont;

                if (scheme_cdr(rands) == SCHEME_NIL)
                    error("define: too few expressions");

                expr = scheme_car(scheme_cdr(rands));

                goto EVAL_EXPRESSION;
            }
                
            case SCHEME_SETX: {
                continuation_t* old_cont = cont;
                
                if (rands == SCHEME_NIL)
                    error("set!: too few expressions");

                cont = &assign_cont;
                cont->data.assignment.env = env;
                cont->data.assignment.var = scheme_car(rands);
                cont->data.assignment.cont = old_cont;

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
            continuation_t* old_cont = cont;
            expr = rator;
            cont = &rator_cont;
            cont->data.eval_rator.rands = rands;
            cont->data.eval_rator.env = env;
            cont->data.eval_rator.cont = old_cont;

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
        return val;

    case TEST:
        if (val != SCHEME_FALSE)
            expr = cont->data.test.true_expr;
        else
            expr = cont->data.test.false_expr;
        env = cont->data.test.env;
        cont = cont->data.test.cont;

        goto EVAL_EXPRESSION;
    
    case VARASSIGN:
        if (env_lookup(cont->data.assignment.env,
                       cont->data.assignment.var, NULL)) {
            env_bind(cont->data.assignment.env,
                     cont->data.assignment.var, val);
            val = SCHEME_UNSPEC;
            cont = cont->data.assignment.cont;

            goto APPLY_CONT;
        }
        else
            error("set!: variable not bound");
        
    case DEFINITION:
        env_bind(cont->data.assignment.env,
                 cont->data.assignment.var, val);
        val = SCHEME_UNSPEC;
        cont = cont->data.assignment.cont;

        goto APPLY_CONT;
        
    case PRIM_ARGS:
        cont = cont->data.prim_args.cont;
        // val = apply_primative(cont->data.prim_args.prim, val);
        error("Primatives don't exist yet");

        goto APPLY_CONT;

    case BEGIN:
        expr = scheme_car(cont->data.eval_begin.exprs);
        cont->data.eval_begin.exprs =
            scheme_cdr(cont->data.eval_begin.exprs);

        if (cont->data.eval_begin.exprs == SCHEME_NIL)
            cont = &halt_cont;
        
        goto EVAL_EXPRESSION;
        
    case EVAL_RATOR: {
        continuation_t* old_cont = cont->data.eval_rator.cont;
        
        rands = cont->data.eval_rator.rands;
        env = cont->data.eval_rator.env;
        cont = &rands_cont;
        cont->data.eval_rands.proc = val;
        cont->data.eval_rands.cont = old_cont;

        goto EVAL_RANDS;
    }

    case EVAL_RANDS:
        if (IS_PROCEDURE(cont->data.eval_rands.proc)) {
            proc = cont->data.eval_rands.proc;
            args = val;
            cont = cont->data.eval_rands.cont;

            goto APPLY_PROCVAL;
        }
        else
            error("Attempt to apply non-procedure");

    case EVAL_FIRST: {
        continuation_t* old_cont = cont->data.eval_first.cont;
        
        rands = scheme_cdr(cont->data.eval_first.exprs);
        env = cont->data.eval_first.env;
        cont = &rest_cont;
        cont->data.eval_rest.first_value = val;
        cont->data.eval_rest.cont = old_cont;

        goto EVAL_RANDS;
    }

    case EVAL_REST:
        val = scheme_cons(cont->data.eval_rest.first_value, val);
        cont = cont->data.eval_rest.cont;

        goto APPLY_CONT;
    }

 EVAL_RANDS:
    if (rands == SCHEME_NIL) {
        val = SCHEME_NIL;
        goto APPLY_CONT;
    }
    else {
        continuation_t* old_cont = cont;
        expr = scheme_car(rands);
        cont = &first_cont;
        cont->data.eval_first.exprs = rands;
        cont->data.eval_first.env = env;
        cont->data.eval_first.cont = old_cont;

        goto EVAL_EXPRESSION;
    }
    
 APPLY_PROCVAL: {
        continuation_t* old_cont = cont;
        struct procedure* p = GET_PROCEDURE(proc);
        scheme_t vars = p->formals;
        scheme_t vals = args;
        
        /*
         * Extend lexical environment with formal parameters
         */
        env = p->env;
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
        expr = scheme_cons(make_symbol("begin", 5), p->body);

        goto EVAL_EXPRESSION;
    }
}
