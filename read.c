/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <readline/readline.h>
#include "scheme.h"
#include "lex.yy.c"

char* prompt0 = "> ";
char* prompt1 = "? ";

/*
 * Really primitive and bogus number parsing.  Only parses fixnums
 * (but in any base)
 */
scheme_t read_number(char* str, size_t n)
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

scheme_t read_token(int token, char* str, size_t n)
{
    switch (token) {
    case IDENTIFIER:
        return make_symbol(str, n);
        
    case BOOLEAN: {
        switch (str[1]) {
        case 't':
        case 'T':
            return SCHEME_TRUE;
        case 'f':
        case 'F':
            return SCHEME_FALSE;
        default:
            printf("INTERNAL ERROR: Can't parse boolean\n");
            abort();
        }
    }
        
    case NUMBER:
        return read_number(yytext, yyleng);
        
    case CHARACTER: {
        if (n > 3) {    /* It's a character name */
            if (strcasecmp(str+2, "space") == 0)
                return MAKE_CHAR(' ');
            else if (strcasecmp(str+2, "newline") == 0)
                return MAKE_CHAR('\n');
            else {
                printf("Unknown character name");
                return SCHEME_NIL;
            }
        }
        else
            return MAKE_CHAR(str[2]);
    }
        
    case STRING:
        return make_string(buf, buflen);
        
    default:
        printf("INTERNAL ERROR: Unknown token %d\n");
        abort();
    }

    return SCHEME_NIL;

}

/*
 * Read and construct a pair
 * FIXME: Do this iteratively, not recursively
 */
scheme_t read_pair()
{
    int token = yylex();
    scheme_t cdr;
    
    if (token == RP)
        return SCHEME_NIL;
    else if (token == LP) {
        scheme_t car = read_pair();
        scheme_t cdr = read_pair();
        return scheme_cons(car, cdr);
    }
    else if (token == YY_NULL) {
        printf("ERROR: Unbalanced parentheses\n");
        return SCHEME_UNSPEC;
    }

    else {
        scheme_t car = read_token(token, yytext, yyleng);
        scheme_t cdr = read_pair();
        return scheme_cons(car, cdr);
    }
}


scheme_t scheme_read() {
    int token;
    /*
     * yylex() returns the token type, the lexeme and its length are
     * available in yytext and yyleng, respectively.  In the case of a
     * STRING token, the string is in buf.
     */

    token = yylex();

    switch(token) {
    case LP:
        return read_pair();

    case RP:
        printf("ERROR: Unexpected ')'\n");
        return SCHEME_UNSPEC;
        
    default:
        return read_token(token, yytext, yyleng);
    }

}

int main(int argc, char* argv[])
{
    char* line;
    
    while ((line = readline(prompt0)) != NULL) {
	int token;
	scheme_t s;
	YY_BUFFER_STATE yybuf = yy_scan_string(line);
	
	if (strlen(line) > 0) {
            s = scheme_read();
            scheme_write(s);
            printf("\n");
	    add_history(line);
        }
    }
    
    return 0;
}
