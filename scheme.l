/* scanner for R5RS Scheme */

%{
#include "tokens.h"
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
RESERVED		[\[\]{}]

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

<STR>\\\"		{ yymore(); }
<STR>\\\\		{ yymore(); }
<STR>[^\\\"]*		{ yymore();  }
<STR>\"			{ BEGIN 0; return STRING; }
\"			{ BEGIN STR; }

{RESERVED}		{ printf("Reserved character: %s\n", yytext); return ERROR; }
.			{ printf("Unrecognized character: %s\n", yytext); return ERROR; }

%%

