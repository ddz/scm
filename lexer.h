/* $Id$ */
/*
 * Scheme Lexer
 */

#ifndef LEXER_H
#define LEXER_H

#include <sys/types.h>

#define IDENTIFIER 1    /* Symbol or syntactic keyword  */
#define BOOLEAN    2    /* #t, #f                       */
#define NUMBER     3    /* 10, #b10101, #xdeadbeef, ... */
#define CHARACTER  4    /* #\1, \#space, ...            */
#define STRING     5    /* "Blah \\ \" Blah "           */
#define LP         6    /* Left parenthesis '('         */
#define RP         7    /* Right parenthesis ')'        */
#define SP         8    /* Sharp parenthesis '#('       */
#define QUOTE      9    /* , -> (quote ...)             */
#define BACKQUOTE 10    /* ` -> (quasiquote ...)        */
#define COMMA     11    /* , -> (unquote ...)           */
#define COMMAAT   12    /* ,@ -> (unquote-splicing ...) */
#define PERIOD    13    /* (a . b)                      */

#define ERROR     15

typedef struct {
    int    token;
    char*  lexeme;
    size_t length;
} token_t;

extern int lexer_scan_string(char* str);
extern int lexer_scan_file(FILE* f);
extern int lexer_next_token(token_t* t);
extern int lexer_peek_token(token_t* t);

#endif
