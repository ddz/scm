/* $Id$ */
/*
 * Scheme top-level read-eval-print loop
 */

#include "scheme.h"

jmp_buf top_level;
char* error_msg;
env_frame_t* top_env = NULL;

void init_env()
{
    top_env = make_environment(NULL);
    env_bind(top_env, make_symbol("quote", 5), SCHEME_QUOTE);
    env_bind(top_env, make_symbol("lambda", 6), SCHEME_LAMBDA);
    env_bind(top_env, make_symbol("if", 2), SCHEME_IF);
    env_bind(top_env, make_symbol("set!", 4), SCHEME_SETX);
    env_bind(top_env, make_symbol("quasiquote", 9), SCHEME_QUASIQUOTE);
    env_bind(top_env, make_symbol("define", 6), SCHEME_DEFINE);
    env_bind(top_env, make_symbol("unquote", 7), SCHEME_UNQUOTE);
    env_bind(top_env, make_symbol("quote-splicing", 15),
             SCHEME_UNQUOTE_SPLICING);
}

int main(int argc, char* argv[])
{
    scheme_t s, e;

    init_env();
    while (1) {
        printf("> ");
        if (setjmp(top_level) == 0) {
            if ((s = scheme_read(stdin)) == SCHEME_EOF)
                break;
            e = scheme_eval(s, top_env);
            if (e == SCHEME_UNSPEC)
                continue;
            scheme_write_1(e);
        }
        else {
	    char c;
	    while ((c = getc(stdin)) != EOF)
		if (c == '\n')
		    break;
            printf("ERROR: %s", error_msg);
	}
        printf("\n");
    }
    return 0;
}
