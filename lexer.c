/* $Id$ */
#include <stdio.h>
#include "lexer.h"
#include "lex.yy.c"

/*
 * The Scheme lexer currently just wraps tokens returned by lex.  This
 * is just to introduce the additional overhead of extra functions
 * calls (actually the use of lex may go away someday).
 */

static YY_BUFFER_STATE current_state;

/*
 * Begin scanning a string
 */
int lexer_scan_string(char* str)
{
    current_state = yy_scan_string(str);
    return 0;
}

/*
 * Begin scanning a file
 */
int lexer_scan_file(FILE* f)
{
    current_state = yy_create_buffer(f, YY_BUF_SIZE);
    yy_switch_to_buffer(current_state);

    return 0;
}

/*
 * Return the next token from the input stream
 */
int lexer_next_token(token_t* t)
{
    t->token = yylex();
    t->lexeme = yytext;
    t->length = yyleng;

    return 0;
}

/*
 * Peek at the next token from the input stream
 */
int lexer_peek_token(token_t* t)
{
    t->token = yylex();
    t->lexeme = yytext;
    t->length = yyleng;

    yyless(0);
    
    return 0;
}
