/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <setjmp.h>

#include "scheme.h"
#include "lexer.h"
#include "stk.h"
#include "strbuf.h"

#define error(msg) do { error_msg = msg; longjmp(top_level, 1); } while (0);

jmp_buf top_level;
char* error_msg;

static void     init();
static void     repl();
static scheme_t read_datum(int);
static scheme_t read_list(int);
static scheme_t read_vector(int);
static scheme_t read_simple(token_t*);
static scheme_t read_string(token_t*);
static scheme_t read_number(token_t*);

int main(int argc, char* argv[])
{
    init();
    repl();
}

void init()
{
    lexer_scan_file(stdin);
}

void repl(void)
{
    scheme_t s;

    printf("> ");
    while (1) {
	if (setjmp(top_level) == 0) {
	    s = scheme_read_0();
	    if (s == SCHEME_EOF)
		break;
	    scheme_write_1(s);
	}
	else {
	    printf("ERROR: %s", error_msg);
	}
	printf("\n> ");
    }
}

scheme_t scheme_current_input_port()
{
    return SCHEME_NIL;
}

scheme_t scheme_read_0()
{
    return scheme_read_1(scheme_current_input_port());
}

scheme_t scheme_read_1(scheme_t port)
{
    return read_datum(0);
}


/*
 * Read a Scheme datum (just about any scheme object)
 */
scheme_t read_datum(int depth)
{
    token_t token;

    lexer_next_token(&token);

    switch (token.token) {
    case 0:
        return SCHEME_EOF;
	
    case RP:
	if (depth)
	    return SCHEME_NIL;
	else
	    error("Unexpected ')'");
        
    case LP:
	return read_list(depth + 1);

    case SP:
	return read_vector(depth + 1);

    case PERIOD:
        /* XXX: ERROR */
	error("Error: Unexpected '.'");
        
    case QUOTE:
	return scheme_cons(SCHEME_QUOTE,
			   scheme_cons(read_datum(depth + 1),
				       SCHEME_NIL));
    case BACKQUOTE:
	return scheme_cons(SCHEME_QUASIQUOTE,
			   scheme_cons(read_datum(depth + 1),
				       SCHEME_NIL));
    case COMMA:
        return scheme_cons(SCHEME_UNQUOTE,
			   scheme_cons(read_datum(depth + 1),
				       SCHEME_NIL));
    case COMMAAT:
        return scheme_cons(SCHEME_UNQUOTE_SPLICING,
			   scheme_cons(read_datum(depth + 1),
				       SCHEME_NIL));
    }
    
    return read_simple(&token);
}

/*
 * Read a list of Scheme objects
 */
scheme_t read_list(int depth)
{
    stk_t    stk = STK_INITIALIZER;
    scheme_t s, ls = SCHEME_NIL;

    while (1) {
	token_t token;
	
        if ((s = read_datum(depth + 1)) == SCHEME_UNSPEC) {
	    error("Unbalanced parentheses (missing ')')");
	}

        /*
         * read_datum() returns SCHEME_NIL to signify end-of-list.
         */
        if (s == SCHEME_NIL)
            break;
        
        stk_push(&stk, (void*)s);

        lexer_peek_token(&token);
	if (token.token == PERIOD) {
            lexer_next_token(&token);    /* Eat '.' token */
            
	    ls = read_datum(depth + 1);

            /*
             * Ensure that the . was before the last list element.
             */
            lexer_peek_token(&token);
            if (token.token != RP) {
                error("Unexpected '.'");
            }
            lexer_next_token(&token);    /* Eat ')' token */
            
	    break;
	}
    }

    while (!stk_empty(&stk)) {
        scheme_t car = (scheme_t)stk_pop(&stk);
        ls = scheme_cons(car, ls);
    }

    return ls;
}

/*
 * Read a vector of Scheme objects
 */
scheme_t read_vector(int depth)
{
    int i = 0, elems = 0;
    stk_t stk = STK_INITIALIZER;
    scheme_t s;
    scheme_t* vector;

    /*
     * Read in all the elements into a stack and then load them
     * backwards into properly sized array of scheme_t.
     */
    
    while ((s = read_datum(depth + 1)) != SCHEME_NIL) {
	elems++;
	stk_push(&stk, (void*)s);
    }

    vector = (scheme_t*)malloc(elems * sizeof(scheme_t));

    i = elems;
    while (!stk_empty(&stk))
	vector[--i] = (scheme_t)stk_pop(&stk);

    return MAKE_VECTOR(vector, elems);
}


/*
 * Read a simple datum (boolean, number, character, string, or symbol)
 */
scheme_t read_simple(token_t* token)
{
    switch (token->token) {
    case IDENTIFIER:
        return MAKE_SYMBOL(strdup(token->lexeme), token->length);
        
    case BOOLEAN:
        if (strcasecmp(token->lexeme, "#t") == 0)
            return SCHEME_TRUE;
        else
            return SCHEME_FALSE;
        
    case NUMBER:
        return read_number(token);

    case CHARACTER:
        if (strncasecmp(token->lexeme + 2, "space", 5) == 0)
            return MAKE_CHAR(' ');
        else if (strncasecmp(token->lexeme + 2, "newline", 7) == 0)
            return MAKE_CHAR('\n');
        else
            return MAKE_CHAR(token->lexeme[2]);

    case STRING:
        return read_string(token);
    }
    
    error("Unknown simple datum token");
    
    return SCHEME_UNSPEC;    /* Never reached */
}



/*
 * Read in a Scheme string
 */
scheme_t read_string(token_t* token)
{
    char* str = token->lexeme;
    int   len = token->length;
    strbuf_t sb;
    int i;
    
    strbuf_init(&sb);
    
    for (i = 1; i < len; i++) {
        if (str[i] == '\\') {
            if (str[i + 1] == '\\')
                strbuf_append(&sb, "\\", 1);
            else if (str[i + 1] == '\"')
                strbuf_append(&sb, "\"", 1);
            else {
                error("Unknown string escape");
            }
            i++;
        }
        else if (str[i] == '\"')
            break;
        else
            strbuf_append(&sb, str + i, 1);
    }
    
    return MAKE_STRING(strbuf_buffer(&sb), strbuf_length(&sb));
}

/*
 * Read in a Scheme number
 */
scheme_t read_number(token_t* token)
{
    char* str = token->lexeme;
    scheme_t num;
    char* endptr = NULL;
    int i = -1, base = 10, exact = 0, inexact = 0;

    /*
     * Read in any optional base or exactness flags
     */
    while (str[0] == '#') {
        switch (str[1]) {
        case 'b':
            base = 2;
            break;
        case 'o':
            base = 8;
            break;
        case 'd':
            base = 10;
            break;
        case 'x':
            base = 16;
            break;
        case 'i':
            inexact = 1;
            break;
        case 'e':
            exact = 1;
            break;
        default:
	    error("unknown base or exactness");
        }
        
        str += 2;
    }

    i = strtol(str, &endptr, base);
    if (errno == ERANGE) {
        if (i == LONG_MIN) {
            error("strtol underflow");
        }
        else if (i == LONG_MAX) {
            error("strtol overflow");
        }
    }
    else if (*endptr) {
        error("illegal character");
    }
        
    num = MAKE_FIXNUM(i);
    if (GET_FIXNUM(num) != i) {
        error("fixnum overflow");
    }

    return num;
}

