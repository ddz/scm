/* scanner for R5RS Scheme */

%{
#include "tokens.h"
#include "types.h"

char* buf = NULL;
size_t bufsize = 0;
size_t buflen = 0;

void string_init();
void string_add(char*, size_t);
    
%}

WHITESPACE		[ \n]
LETTER			[a-zA-Z]
SPECIALINITIAL		[!$%&*/:<=>?^_~]
INITIAL			[a-zA-Z!$%&*/:<=>?^_~]
DIGIT			[0-9]
DIGIT2			[01]
DIGIT8			[0-7]
DIGIT16			[0-9a-fA-F]
SPECIALSUBSEQUENT	[+\-.@]
SUBSEQUENT		[a-zA-Z!$%&*/:<=>?^_~0-9+\-.@]
NUMCHARS                [0-9a-fA-F+\-bodxieesfdl.#@]

%x STR

%%

{WHITESPACE}		/* Eat whitespace */
;.*			/* Eat comments */

"#("			{ return SP; }
"("			{ return LP; }
")"			{ return RP; }
"'"			{ return QUOTE; }
"`"			{ return BACKQUOTE; }
",@"			{ return COMMAAT; }
","			{ return COMMA; }
"."			{ return PERIOD; }

{INITIAL}{SUBSEQUENT}*	|
"+"|"-"|"..."		{ return IDENTIFIER; }
	
"#t"|"#f"		{ return BOOLEAN; }

#[bodx]{NUMCHARS}+	|
#[ie]{NUMCHARS}+	|
{DIGIT}{NUMCHARS}*	|
"+"{NUMCHARS}+		|
"-"{NUMCHARS}+		{ return NUMBER; }

#\\[^)]+       		{ return CHARACTER; }


<STR>\\\"		{ string_add("\"", 1); }
<STR>\\\\		{ string_add("\\", 1); }
<STR>[^\\\"]*		{ string_add(yytext, yyleng);  }
<STR>\"			{ BEGIN 0; return STRING; }
\"			{ BEGIN STR; string_init(); }

"[" | "]" | "{" | "}"	{ printf("Reserved character: %s\n", yytext); }
.			{ printf("Unrecognized character: %s\n", yytext); }

%%

/*
 * Initialize or reinitialize string buffer
 */
void string_init()
{
    if (buf == NULL) {
	if ((buf = malloc(64)) == NULL) {
	    printf("INTERNAL ERROR: Couldn't allocate string\n");
	    abort();
	}
	
	bufsize = 64;
	buflen = 0;
    }
    else {
	buflen = 0;
	buf[0] = '\0';
    }
}

/*
 * Add a chunk to the string buffer, growing it if necessary
 */
void string_add(char* s, size_t n)
{
    if (buflen + n + 1 > bufsize) {
	bufsize *= 2;
	if ((buf = realloc(buf, bufsize)) == NULL) {
	    printf("INTERNAL ERROR: Couldn't reallocate string\n");
	    abort();
	}
    }

    strncat(buf, s, n);
    buflen += n;
}
