/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <setjmp.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "scheme.h"
#include "lexer.h"
#include "stk.h"
#include "strbuf.h"

char* prompt0 = "> ";
char* prompt1 = "? ";

jmp_buf top_level;

static scheme_t read_datum();
static scheme_t read_list();
static scheme_t read_vector();
static scheme_t read_simple(token_t*);
static scheme_t read_string(token_t*);
static scheme_t read_number(token_t*);

int main(int argc, char* argv[])
{
    char* line;
    
    while ((line = readline(prompt0)) != NULL) {
        if (strlen(line) == 0)
            continue;

        /*
         * Begin scanning the line for tokens
         */
        lexer_scan_string(line);

        /*
         * Set a jmp point so errors return to the top-level
         */
        if (!setjmp(top_level)) {
            scheme_t s;
	    if ((s = read_datum()) != SCHEME_UNSPEC) {
		token_t token;
		lexer_peek_token(&token);
		if (token.token == RP) {
		    printf("ERROR: Unbalanced parenthesis\n");
		    longjmp(top_level, 1);
		}
                scheme_write_1(s);
                printf("\n");
            }
            
            add_history(line);
        }
    }
    
    return 0;
}

/*
 * Read a Scheme datum (just about any scheme object)
 */
scheme_t read_datum()
{
    token_t token;

    lexer_next_token(&token);

    switch (token.token) {
    case ERROR:
	longjmp(top_level, 1);
	
    case 0:
        return SCHEME_UNSPEC;
    case RP:
        return SCHEME_NIL;
        
    case LP:
	return read_list();

    case SP:
	return read_vector();

    case PERIOD:
        /* XXX: ERROR */
	printf("Error: Unexpected '.'\n");
        break;
        
    case QUOTE:
	return scheme_cons(SCHEME_QUOTE, read_list());
    case BACKQUOTE:
	return scheme_cons(SCHEME_QUASIQUOTE, read_list());
    case COMMA:
        return scheme_cons(SCHEME_UNQUOTE, read_list());
    case COMMAAT:
        return scheme_cons(SCHEME_UNQUOTE_SPLICING, read_list());
    }
    
    return read_simple(&token);
}

/*
 * Read a list of Scheme objects
 */
scheme_t read_list()
{
    stk_t    stk = STK_INITIALIZER;
    scheme_t s, ls = SCHEME_NIL;

    while (1) {
	token_t token;
	
        if ((s = read_datum()) == SCHEME_UNSPEC) {
	    printf("ERROR: Unbalanced parentheses (missing ')')\n");
	    longjmp(top_level, 1);
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
            
	    ls = read_datum();

            /*
             * Ensure that the . was before the last list element.
             */
            lexer_peek_token(&token);
            if (token.token != RP) {
                printf("ERROR: Unexpected '.'\n");
                longjmp(top_level, 1);
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
scheme_t read_vector()
{
    int i = 0, elems = 0;
    stk_t stk = STK_INITIALIZER;
    scheme_t s;
    scheme_t* vector;

    /*
     * Read in all the elements into a stack and then load them
     * backwards into properly sized array of scheme_t.
     */
    
    while ((s = read_datum()) != SCHEME_NIL) {
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
    
    printf("ERROR: Unknown simple datum token\n");
    longjmp(top_level, 1);
    
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
                printf("ERROR: Unknown string escape\n");
                longjmp(top_level, 1);
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
            printf("read_number: unknown base or exactness\n");
	    longjmp(top_level, 1);
        }
        
        str += 2;
    }

    i = strtol(str, &endptr, base);
    if (errno == ERANGE) {
        if (i == LONG_MIN) {
            printf("read_number: strtol underflow\n");
	    longjmp(top_level, 1);
        }
        else if (i == LONG_MAX) {
            printf("read_number: strtol overflow\n");
	    longjmp(top_level, 1);
        }
    }
    else if (*endptr) {
        printf("read_number: illegal character (%c)\n", *endptr);
	longjmp(top_level, 1);
    }
        
    num = MAKE_FIXNUM(i);
    if (GET_FIXNUM(num) != i) {
        printf("read_number: fixnum overflow\n");
	longjmp(top_level, 1);
    }

    return num;
}

