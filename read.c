/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <setjmp.h>
#include <readline/readline.h>

#include "scheme.h"
#include "lex.yy.c"
#include "que.h"
#include "stk.h"

char* prompt0 = "> ";
char* prompt1 = "? ";

jmp_buf top_level;

static scheme_t read_datum();
static scheme_t read_list();
static scheme_t read_vector();
static scheme_t read_simple(char* str, int len);
static scheme_t read_number(char* str, int len);

scheme_t read_number(char* str, int n)
{
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
            return SCHEME_UNSPEC;
        }
        
        str += 2;
    }

    i = strtol(str, &endptr, base);
    if (errno == ERANGE) {
        if (i == LONG_MIN) {
            printf("read_number: strtol underflow\n");
            return SCHEME_UNSPEC;
        }
        else if (i == LONG_MAX) {
            printf("read_number: strtol overflow\n");
            return SCHEME_UNSPEC;
        }
    }
    else if (*endptr) {
        printf("read_number: illegal character (%c)\n", *endptr);
        return SCHEME_UNSPEC;
    }
        
    num = MAKE_FIXNUM(i);
    if (GET_FIXNUM(num) != i) {
        printf("read_number: fixnum overflow\n");
        return SCHEME_UNSPEC;
    }

    return num;
}

/*
 * Read a simple datum (boolean, number, character, string, or symbol)
 */
scheme_t read_simple(char* str, int len)
{
    if (str[0] == '#') {
	switch (str[1]) {
	case 't':
	    return SCHEME_TRUE;
	case 'f':
	    return SCHEME_FALSE;
	case 'b':
	case 'o':
	case 'd':
	case 'x':
	case 'i':
	case 'e':
	    return read_number(str, len);
	case '\\':
	    if (strncasecmp(str + 2, "space", 5) == 0)
		return MAKE_CHAR(' ');
	    else if (strncasecmp(str + 2, "newline", 7) == 0)
		return MAKE_CHAR('\n');
	    else
		return MAKE_CHAR(str[2]);
	}
    }
    else if (str[0] == '\"') {
	int i;
	
	for (i = 1; i++; str[i]) {
	    switch (str[i]) {
	    case '\"':
		break;
	    case '\\':
		break;
	    default:
		break;
	    }
	}

	return MAKE_STRING("BOGUS", 5);
    }

    else if (isdigit(str[0]))
	return read_number(str, len);
    
    else {
	printf("ERROR: Unknown simple datum <%s>\n", str);
	return SCHEME_UNSPEC;
    }
}

scheme_t read_datum()
{
    int token = yylex();

    switch (token) {
    case ERROR:
	longjmp(top_level, 0);
	
    case 0:
    case RP:
        return SCHEME_NIL;
        
    case LP:
	return read_list();

    case SP:
	return read_vector();

    case PERIOD:
	printf("Error: Unexpected '.'\n");
        break;
        
    case QUOTE:
	return SCHEME_CONS(SCHEME_QUOTE, read_list());
    case BACKQUOTE:
	return SCHEME_CONS(SCHEME_QUASIQUOTE, read_list());
    case COMMA:
        return SCHEME_CONS(SCHEME_UNQUOTE, read_list());
               
    case COMMAAT:
        return SCHEME_CONS(SCHEME_UNQUOTE_SPLICING, read_list());
               
    default:
	return read_simple(yytext, yyleng);
    }
    
}

scheme_t read_list()
{
    stk_t    stk = STK_INITIALIZER;
    scheme_t s, ls = SCHEME_NIL;

    while (1) {
	int token;
	
        s = read_datum();
        if (s == SCHEME_NIL)
            break;
        stk_ins(&stk, (void*)s);

	token = yylex();
	if (token == PERIOD) {
	    ls = read_datum();
	    break;
	}
	else
	    yyless(0);
    }

    while (!stk_empty(&stk)) {
        scheme_t car = (scheme_t)stk_rem(&stk);
        ls = SCHEME_CONS(car, ls);
    }

    if (ls == SCHEME_NIL)
	return SCHEME_CONS(SCHEME_NIL, SCHEME_NIL);
    return ls;
}

scheme_t read_vector()
{
    int i = 0, elems = 0;
    que_t que = QUE_INITIALIZER;
    scheme_t s;
    scheme_t* vector;

    while ((s = read_datum()) != SCHEME_NIL) {
	elems++;
	que_ins(&que, (void*)s);
    }

    vector = (scheme_t*)malloc(elems * sizeof(scheme_t));

    while (!que_empty(&que))
	vector[i++] = (scheme_t)que_rem(&que);

    return MAKE_VECTOR(vector, elems);
}

int main(int argc, char* argv[])
{
    char* line;
    
    while ((line = readline(prompt0)) != NULL) {
	if (strlen(line) > 0) {
            YY_BUFFER_STATE state = yy_scan_string(line);
	    if (!setjmp(top_level)) {
		scheme_t s = read_datum();    /* Read */
		/* Eval */
		scheme_write(s);               /* Print */
		add_history(line);
		printf("\n");
	    }
        }
    }
    
    return 0;
}
