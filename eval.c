/* $Id$ */
/*
 * Scheme evaluator
 */

#include "scheme.h"

static scheme_t eval_each(scheme_t s, env_frame_t* env);
static scheme_t quasi_eval(scheme_t ls, env_frame_t* e);

scheme_t scheme_eval(scheme_t sexpr, env_frame_t* env)
{
    /*
     * A scheme symbol evaluates to what it is bound to in the current
     * environment.
     */
    if (IS_SYMBOL(sexpr)) {
        scheme_t s;
        if (env_lookup(env, sexpr, &s))
            return s;
        error("reference to undefined identifier");
    }
    else if (scheme_pairp(sexpr)) {
        scheme_t op, args;

        op = scheme_eval(scheme_car(sexpr), env);
        args = scheme_cdr(sexpr);
        
        /*
         * Special forms must be evaluated without eval'ing arguments
         */
        if (IS_SYNT(op)) {
            switch (op) {
            case SCHEME_LAMBDA: {
                scheme_t formals, body;

                formals = scheme_car(args);
                body = scheme_cdr(args);
                
                if (body == SCHEME_NIL)
                    error("lambda: Missing body");

                return make_procedure(env, formals, body);
            }
                
            case SCHEME_QUOTE: {
                scheme_t r = scheme_car(args);
                if (scheme_cdr(args) != SCHEME_NIL)
                    error("quote: too many arguments");
                return r;
            }
                
            case SCHEME_IF: {
                scheme_t test, clauses, consequent, alternate;

                if (args == SCHEME_NIL)
                    error("if: too few arguments");
                
                test = scheme_car(args);
                clauses = scheme_cdr(args);

                if (clauses == SCHEME_NIL)
                    error("if: too few arguments");
                
                if (scheme_eval(test, env) != SCHEME_FALSE) {
                    consequent = scheme_car(clauses);
                    return scheme_eval(consequent, env);
                }
                else {
                    scheme_t s = scheme_cdr(clauses);
                    if (s == SCHEME_NIL)
                        return SCHEME_UNSPEC;
                    alternate = scheme_car(s);
                    return scheme_eval(alternate, env);
                }
            }
                
            case SCHEME_DEFINE: {
                scheme_t variable, rest, expression;

                if (args == SCHEME_NIL)
                    error("define: too few expressions");
                
                variable = scheme_car(args);

                if (!IS_SYMBOL(variable))
                    error("define: variable must be a symbol");
                
                rest = scheme_cdr(args);

                if (rest == SCHEME_NIL)
                    error("define: too few expressions");
                
                expression = scheme_car(rest);

                if (scheme_cdr(rest) != SCHEME_NIL)
                    error("define: too many expressions");

                env_bind(env, variable, scheme_eval(expression, env));

                return SCHEME_UNSPEC;
            }
                
            case SCHEME_SETX: {
                scheme_t variable, rest, expression;
                scheme_t value;
                
                if (args == SCHEME_NIL)
                    error("set!: too few expressions");
                    
                variable = scheme_car(args);
                rest = scheme_cdr(args);

                if (rest == SCHEME_NIL)
                    error("set!: too few expressions");

                expression = scheme_car(rest);

                if (scheme_cdr(rest) != SCHEME_NIL)
                    error("set!: too many expressions");
                    
                if (env_lookup(env, variable, &value)) {
                    env_bind(env, variable, scheme_eval(expression, env));
                    return SCHEME_UNSPEC;
                }

                error("set!: variable not bound");
            }
                
            case SCHEME_QUASIQUOTE:
                if (scheme_cdr(args) != SCHEME_NIL)
                    error("quasiquote: too many expressions");
		return quasi_eval(scheme_car(args), env);
		
            case SCHEME_UNQUOTE:
            case SCHEME_UNQUOTE_SPLICING:
                error("Not in quasiquote");
                
            default:
                error("Unknown syntactic form\n");
            }
        }
        else {
            scheme_t eval_args = eval_each(args, env);
            return scheme_apply_2(op, eval_args);
        }
    }

    return sexpr; // Everything else is self-evaluating
}

scheme_t eval_each(scheme_t ls, env_frame_t* env)
{
    if (ls == SCHEME_NIL)
        return SCHEME_NIL;
    
    return scheme_cons(scheme_eval(scheme_car(ls), env),
                       eval_each(scheme_cdr(ls), env));
}

scheme_t scheme_apply_2(scheme_t operator, scheme_t operands)
{
    struct procedure* proc;
    scheme_t vars, vals, body, r;
    env_frame_t* env;

    if (!IS_PROCEDURE(operator))
	error("apply: Operator must be a procedure.");
    
    proc = GET_PROCEDURE(operator);
    vars = proc->formals;
    vals = operands;
    env = make_environment(proc->env);
    body = proc->body;
    
    /*
     * Bind arguments to procedure parameters
     */
    while (vars != SCHEME_NIL &&
           vals != SCHEME_NIL &&
           IS_PAIRPTR(vars)) {
        scheme_t var, val;
        
        if (IS_PAIRPTR(vars))
            var = scheme_car(vars);
        else
            var = vars;
        val = scheme_car(vals);
        
        env_bind(env, var, val);

        vars = scheme_cdr(vars);
        vals = scheme_cdr(vals);
    }
    if (IS_SYMBOL(vars))
        env_bind(env, vars, vals);

    /*
     * Evaluate each expression in the body in the newly created
     * environment
     */
    while (body != SCHEME_NIL) {
        scheme_t expr = scheme_car(body);
        body = scheme_cdr(body);
        r = scheme_eval(expr, env);
    }

    /*
     * Return the value of the last expression (like an implicit
     * begin)
     */
    return r;
}

scheme_t quasi_eval(scheme_t s, env_frame_t* env)
{
    if (scheme_pairp(s)) {
        scheme_t a = scheme_car(s);
        scheme_t d = scheme_cdr(s);

        if (IS_SYNT(a)) {
            if (a == SYNT_UNQUOTE) {
                scheme_t s = scheme_car(d);

                if (scheme_cdr(d) != SCHEME_NIL)
                    error("unquote: too many expressions\n");
                
                return scheme_eval(s, env);
            }
            else if (a == SYNT_UNQUOTE_SPLICING) {
                error("unquote-splicing: wrong context\n");
            }
        }

        else if (scheme_pairp(a)) {
            scheme_t aa = scheme_car(a);
            scheme_t ad = scheme_cdr(a);

            if (IS_SYNT(aa)) {
                if (aa == SYNT_UNQUOTE_SPLICING) {
                    scheme_t us = scheme_car(ad);

                    if (scheme_cdr(ad) != SCHEME_NIL)
                        error("unquote-splicing: too many expressions\n");
                
                    us = scheme_eval(us, env);

                    if (!scheme_pairp(us))
                        error("unquote-splicing: must eval to list\n");

                    return scheme_append(us, quasi_eval(d, env));
                }
            }
        }
        
        return scheme_cons(quasi_eval(a, env),
                           quasi_eval(d, env));
    }

    return s;
}
