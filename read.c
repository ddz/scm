/* $Id$ */
#include <stdio.h>
#include <readline/readline.h>
#include "lex.yy.c"

char* prompt0 = "> ";
char* prompt1 = "? ";

scheme_t scheme_read() {
    int token;
    char* token_names[] = {
	"NULL", "IDENTIFIER", "BOOLEAN", "NUMBER", "CHARACTER",
	"STRING", "LP", "RP", "SP", "QUOTE", "BACKQUOTE", "COMMA",
	"COMMAAT", "PERIOD"
    };
    
    /*
     * yylex() returns the token type, the lexeme and its length are
     * available in yytext and yyleng, respectively.  In the case of a
     * STRING token, the string is in buf.
     */
    
    while ((token = yylex())) {
	printf("Token type: %s (%d)\n", token_names[token], token);
	
        switch (token) {
	case YY_NULL:
	    return SCHEME_NIL;
	    
	case IDENTIFIER:
	    return SCHEME_NIL;
	    
	case BOOLEAN: {
	    switch (yytext[1]) {
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
	    return SCHEME_NIL;
	    
	case CHARACTER: {
	    printf("Character: %s@%d\n", yytext, yyleng);
	    if (yyleng > 3) {    /* It's a character name */
		if (strcasecmp(yytext+2, "space"))
		    return MAKE_CHAR(' ');
		else if (strcasecmp(yytext+2, "newline"))
		    return MAKE_CHAR('\n');
		else {
		    printf("Unknown character name");
		    return SCHEME_NIL;
		}
	    }
	    else
		return MAKE_CHAR(yytext[2]);
	}
	    
	case STRING:
	    return SCHEME_NIL;
	    
	case LP:
	    return SCHEME_NIL;
	case RP:
	    return SCHEME_NIL;
	case SP:
	    return SCHEME_NIL;
	case QUOTE:
	    return SCHEME_NIL;
	case BACKQUOTE:
	    return SCHEME_NIL;
	case COMMA:
	    return SCHEME_NIL;
	case COMMAAT:
	    return SCHEME_NIL;
	case PERIOD:
	    return SCHEME_NIL;
	default:
	    printf("INTERNAL ERROR: Unknown token %d\n");
	    abort();
	}
    }
}

int main(int argc, char* argv[])
{
    char* line;
    
    while ((line = readline(prompt0)) != NULL) {
	int token;
	scheme_t s;
	YY_BUFFER_STATE yybuf = yy_scan_string(line);
	
	if (strlen(line) > 0)
	    add_history(line);

	s = scheme_read();
    }
    
    return 0;
}
