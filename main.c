/* $Id$ */
/*
 * Scheme top-level read-eval-print loop
 */

#include "scheme.h"

jmp_buf top_level;
char* error_msg;

int main(int argc, char* argv[])
{
    scheme_t s;

    while (1) {
        printf("> ");
        if (setjmp(top_level) == 0) {
            if ((s = scheme_read(stdin)) == SCHEME_EOF)
                break;
            scheme_write_1(s);
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
