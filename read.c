/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <readline/readline.h>

#include "scheme.h"
#include "fstropen.h"
#include "que.h"

char* prompt0 = "> ";
char* prompt1 = "? ";

scheme_t read_number(FILE* f)
{
    printf("READ NUMBER: <%s>\n", str);
    return SCHEME_UNSPEC;
}

/*
 * Read a simple datum (boolean, number, character, string, or symbol)
 */
scheme_t read_simple(FILE* f)
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
	    return read_number(str);
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
	return read_number(str);
    
    else {
	printf("ERROR: Unknown simple datum <%s>\n", str);
	return SCHEME_UNSPEC;
    }
}

scheme_t read_list(FILE* f)
{
    printf("READ_LIST");
    return SCHEME_UNSPEC;
}

scheme_t read_vector(FILE* f)
{
    que_t q;

    printf("READ VECTOR");
    return SCHEME_UNSPEC;
}

scheme_t read_datum(FILE* f)
{
    char* tok, *ptr = str;

    while (isspace(str[i])) i++;

    switch (str[i]) {
    case '(':
	return read_list(str, i + 1, SCHEME_NIL);
    case '\'':
	return read_list(str, i + 1,
			 SCHEME_CONS(MAKE_SYMBOL("quote", 5),
				     SCHEME_NIL));
    case '`':
	return read_list(str, i + 1,
			 SCHEME_CONS(MAKE_SYMBOL("quasiquote", 10),
				     SCHEME_NIL));
    case ',':
	if (str[i + 1] == '@')
	    return read_list(str, i + 2,
			     SCHEME_CONS(MAKE_SYMBOL("unquote-splicing", 16),
					 SCHEME_NIL));
	else
	    return read_list(str, i + 1,
			     SCHEME_CONS(MAKE_SYMBOL("unquote", 7),
					 SCHEME_NIL));
    }
	
    if (strncmp(str + i, "#(", 2) == 0)
	return read_vector(str, i);
    else
	return read_simple(str);
}

int main(int argc, char* argv[])
{
    char* line;
    
    while ((line = readline(prompt0)) != NULL) {
	if (strlen(line) > 0) {
	    FILE* f = fstropen(line);
            scheme_t s = read_datum(f);    /* Read */
	    /* Eval */
            scheme_write(s);               /* Print */
	    add_history(line);
            printf("\n");
        }
    }
    
    return 0;
}
