/* $Id$ */
#include <stdio.h>
#include <readline/readline.h>
#include "lex.yy.c"

char* prompt0 = "> ";
char* prompt1 = "? ";

int main(int argc, char* argv[])
{
    char* line;
    while ((line = readline(prompt0)) != NULL) {
	int token;
	YY_BUFFER_STATE yybuf = yy_scan_string(line);
	while ((token = yylex()) != 0) {
	    printf("Token: %d\n", token);
	}
	add_history(line);
    }

    return 0;
}

