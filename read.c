/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <readline/readline.h>

#include "scheme.h"
#include "lex.yy.c"
#include "que.h"
#include "stk.h"

char* prompt0 = "> ";
char* prompt1 = "? ";

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
            return SCHEME_NIL;
        }
        
        str += 2;
    }

    i = strtol(str, &endptr, base);
    if (errno == ERANGE) {
        if (i == LONG_MIN) {
            printf("read_number: strtol underflow\n");
            return SCHEME_NIL;
        }
        else if (i == LONG_MAX) {
            printf("read_number: strtol overflow\n");
            return SCHEME_NIL;
        }
    }
    else if (*endptr) {
        printf("read_number: illegal character (%c)\n", *endptr);
        return SCHEME_NIL;
    }
        
    num = MAKE_FIXNUM(i);
    if (GET_FIXNUM(num) != i) {
        printf("read_number: fixnum overflow\n");
        return SCHEME_NIL;
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
    case 0:
    case RP:
        return SCHEME_NIL;
        
    case LP:
	return read_list();

    case SP:
	return read_vector();

    case PERIOD:
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
        s = read_datum();
        if (s == SCHEME_NIL)
            break;
        stk_ins(&stk, (void*)s);
    }

    while (!stk_empty(&stk)) {
        scheme_t car = (scheme_t)stk_rem(&stk);
        ls = SCHEME_CONS(car, ls);
    }

    return ls;
}

scheme_t read_vector()
{
    que_t que = QUE_INITIALIZER;
    scheme_t s;

    

    return SCHEME_UNSPEC;
}

int main(int argc, char* argv[])
{
    char* line;
    
    while ((line = readline(prompt0)) != NULL) {
	if (strlen(line) > 0) {
            YY_BUFFER_STATE state = yy_scan_string(line);
            scheme_t s = read_datum();    /* Read */
	    /* Eval */
            scheme_write(s);               /* Print */
	    add_history(line);
            printf("\n");
        }
    }
    
    return 0;
}
