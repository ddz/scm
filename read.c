/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include "scheme.h"
#include "stk.h"
#include "strbuf.h"

static scheme_t read_identifier(FILE*);
static scheme_t read_number(FILE*);
static scheme_t read_character(FILE*);
static scheme_t read_string(FILE*);
static int isdelimiter(char);
static int issubsequent(char);
static int isinitial(char);

stk_t read_stk = STK_INITIALIZER;
scheme_t read_tmp = SCHEME_NIL;

sequence_state_t* make_sequence(sequence_type type)
{
    sequence_state_t* seq = malloc(sizeof(sequence_state_t));
    
    seq->dot = seq->abbrev = 0;
    seq->type = type;
    
    if (type == LIST) {
	seq->seq.list.head = seq->seq.list.tail = SCHEME_NIL;
    }
    else if (type == VECTOR) {
	seq->seq.vector.used = 0;
	seq->seq.vector.size = 0;
	seq->seq.vector.v = NULL;
    }

    return seq;
}

scheme_t sequence2scheme(sequence_state_t* seq)
{
    scheme_t s;
    
    if (seq->type == LIST)
	s = seq->seq.list.head;
    else {
	seq->seq.vector.v =
	    realloc(seq->seq.vector.v,
		    seq->seq.vector.used * sizeof(scheme_t));
	s = MAKE_VECTOR(seq->seq.vector.v,
			seq->seq.vector.used);
    }

    return s;
}

void sequence_add(sequence_state_t* seq, scheme_t s)
{
    if (seq->type == LIST) {
	scheme_t c = scheme_cons(s, SCHEME_NIL);
	if (seq->seq.list.tail != SCHEME_NIL)
	    scheme_set_cdrx(seq->seq.list.tail, c);
	else
	    seq->seq.list.head = c;
	seq->seq.list.tail = c;
    }
    else {
	if (seq->seq.vector.used + 1 > seq->seq.vector.size) {
	    seq->seq.vector.size = (seq->seq.vector.size * 2) + 1;
	    seq->seq.vector.v =
		realloc(seq->seq.vector.v,
			seq->seq.vector.size * sizeof(scheme_t));
	}
	
	seq->seq.vector.v[seq->seq.vector.used++] = s;
    }
}

/*
 * Read a scheme token: symbol, boolean, number,
 * character, string, '(', ')', '#(', '\'', '`', ',', ',@', '.'
 *
 * The scheme types are returned as their scheme_t types, the other
 * tokens are returned as newly defined SYNT types.
 */

#define LP       MAKE_SYNT(64)
#define RP       MAKE_SYNT(65)
#define SP       MAKE_SYNT(66)
#define TICK     MAKE_SYNT(67)
#define BACKTICK MAKE_SYNT(68)
#define COMMA    MAKE_SYNT(69)
#define COMMAAT  MAKE_SYNT(70)
#define DOT      MAKE_SYNT(71)

scheme_t read_token(FILE* f)
{
    char c;
    scheme_t s;

    while ((c = getc(f)) != EOF) {
	switch (c) {
	/*
	 * Atmosphere (skip whitespace and comments)
	 */
	case ' ':
	case '\n':
	    continue;
	    
	case ';':
	    do { c = getc(f); } while (c != '\n' && c != EOF);
	    ungetc(c, f);
	    continue;

	case '(':
	    return LP;

	case '.': {
	    if (isdelimiter(ungetc(getc(f), f)))
		return DOT;
	    else {
		ungetc(c, f);
		return read_identifier(f);
	    }
	}
	    
	case ')':
	    return RP;

	case '\"':
	    return read_string(f);

	case ',':
	    return COMMA;

	case '`':
	    return BACKTICK;

	case '\'':
	    return TICK;

	case '+':
	case '-':
	    if (isdelimiter(ungetc(getc(f), f))) {
		ungetc(c, f);
		return read_identifier(f);
	    }
	    else
		return read_number(f);

	case '#': {
	    char d = getc(f);
	    switch (d) {
	    case EOF:
		return SCHEME_EOF;

	    case '(':
		return SP;

	    case 't':
	    case 'T':
		return SCHEME_TRUE;

	    case 'f':
	    case 'F':
		return SCHEME_FALSE;

	    case '\\':
		return read_character(f);

	    default:
		ungetc(d, f);
		ungetc(c, f);
		return read_number(f);
	    }
	}
	
	default:
	    if (isdigit(c)) {
		ungetc(c, f);
		return read_number(f);
	    }
	    else if (isinitial(c)) {
		ungetc(c, f);
		return read_identifier(f);
	    }
	    else
		error("Bad character");
	}
    }
    
    return SCHEME_EOF;
}

scheme_t scheme_read(FILE* f)
{
    sequence_state_t* seq = NULL;

    read_tmp = SCHEME_UNDEF;
    stk_init(&read_stk);
    
    while ((read_tmp = read_token(f)) != SCHEME_EOF) {
	switch (read_tmp) {
	case LP:
	    seq = make_sequence(LIST);
	    stk_push(&read_stk, seq);
	    continue;
	    
	case SP:
	    seq = make_sequence(VECTOR);
	    stk_push(&read_stk, seq);
	    continue;
	    
	case RP:
	    if (stk_empty(&read_stk))
		error("Mismatched parenthesis");
	    seq = stk_pop(&read_stk);
	    read_tmp = sequence2scheme(seq);
	    free(seq);
	    seq = NULL;
	    break;
	    
	case TICK:
	    seq = make_sequence(LIST);
	    stk_push(&read_stk, seq);
	    seq->abbrev = 1;
	    sequence_add(seq, MAKE_SYMBOL("quote", 5));
	    continue;

	case BACKTICK:
	    seq = make_sequence(LIST);
	    stk_push(&read_stk, seq);
	    seq->abbrev = 1;
	    sequence_add(seq, MAKE_SYMBOL("quasiquote", 10));
	    continue;

	case COMMA:
	    seq = make_sequence(LIST);
	    stk_push(&read_stk, seq);
	    seq->abbrev = 1;
	    sequence_add(seq, MAKE_SYMBOL("unquote", 7));
	    continue;

	case COMMAAT:
	    seq = make_sequence(LIST);
	    stk_push(&read_stk, seq);
	    seq->abbrev = 1;
	    sequence_add(seq, MAKE_SYMBOL("unquote-splicing", 16));
	    continue;

	case DOT:
	    if (stk_empty(&read_stk))
		error("Illegal use of \".\"\n");
	    seq = (sequence_state_t*)stk_top(&read_stk);
	    if (seq->type != LIST)
		error("Illegal use of \".\"\n");
	    seq->dot = 1;
	    continue;
	}
	
	if (!stk_empty(&read_stk)) {
	    seq = (sequence_state_t*)stk_pop(&read_stk);

	    if (seq->dot) {
		scheme_set_cdrx(seq->seq.list.tail, read_tmp);
		seq->dot = 0;
	    }
	    else {
		stk_push(&read_stk, seq);
		sequence_add(seq, read_tmp);
		seq = stk_pop(&read_stk);
	    }

	    while (seq && seq->abbrev) {
		read_tmp = sequence2scheme(seq);
		free(seq);
		seq = NULL;
		if (!stk_empty(&read_stk)) {
		    seq = (sequence_state_t*)stk_top(&read_stk);
		    sequence_add(seq, read_tmp);
		    stk_pop(&read_stk);
		}
		else
		    return read_tmp;
	    }

	    if (seq)
		stk_push(&read_stk, seq);
	}
		    
	if (stk_empty(&read_stk))
	    return read_tmp;
    }
}

scheme_t read_identifier(FILE* f)
{
    char c;
    scheme_t s;
    static strbuf_t sb = STRBUF_INITIALIZER;

    while ((c = getc(f)) != EOF) {
	if (issubsequent(c))
	    strbuf_add(&sb, c);
	else {
	    ungetc(c, f);
	    break;
	}
    }

    s = MAKE_SYMBOL(strbuf_buffer(&sb), strbuf_length(&sb));
    strbuf_reset(&sb);

    return s;
}

scheme_t read_number(FILE* f)
{
    char c;
    static strbuf_t sb = STRBUF_INITIALIZER;

    strbuf_reset(&sb);

    while ((c = getc(f)) != EOF) {
	if (isdelimiter(c)) {
	    /*
	     * Use the RnRS string parser, string->number
	     */
	    scheme_t s = MAKE_STRING(strbuf_buffer(&sb),
				     strbuf_length(&sb));
	    ungetc(c, f);
	    return scheme_string2number_1(s);
	}
	else
	    strbuf_add(&sb, c);
    }

    return SCHEME_EOF;
}

scheme_t read_character(FILE* f)
{
    char c;
    
    static strbuf_t sb = STRBUF_INITIALIZER;

    strbuf_reset(&sb);

    c = getc(f);
    if (c == EOF)
	return SCHEME_EOF;
    
    strbuf_add(&sb, c);
    
    while ((c = getc(f)) != EOF) {
	if (isdelimiter(c)) {
	    char* buf = strbuf_buffer(&sb);
	    int length = strbuf_length(&sb);
	    
	    ungetc(c, f);
	    
	    if (length == 1)
		return MAKE_CHAR(buf[0]);
	    else if (strcasecmp(buf, "newline") == 0)
		return MAKE_CHAR('\n');
	    else if (strcasecmp(buf, "space") == 0)
		return MAKE_CHAR(' ');
	    else
		error("Unknown character name");
	}
	else
	    strbuf_add(&sb, c);
    }
    
    return SCHEME_EOF;
}

scheme_t read_string(FILE* f)
{
    char c;
    static strbuf_t sb = STRBUF_INITIALIZER;
    
    while ((c = getc(f)) != EOF) {
        if (c == '"') {
            int n;
            char* str;
            
            str = strdup(sb.buf);
            n = strbuf_length(&sb);
            strbuf_reset(&sb);

            return MAKE_STRING(str, n);
        }
        else if (c ==  '\\') {
            switch (getc(f)) {
            case '\"':
                strbuf_add(&sb, '\"');
                break;
            case '\\':
                strbuf_add(&sb, '\\');
                break;
            default:
                error("Unknown string escape");
            }
        }
        else
            strbuf_add(&sb, c);
    }

    return SCHEME_EOF;
}

int isdelimiter(char c)
{
    static char delims[] = "()\";";

    return (isspace(c) || strchr(delims, c) != NULL);
}

int issubsequent(char c)
{
    static char subsequent[] = "!$%&*/:<=>?^_~+-.@";

    return (isalnum(c) || strchr(subsequent, c) != NULL);
}

int isinitial(char c)
{
    static char specialinitial[] = "!$%&*/:<=>?^_~";

    return (isalpha(c) || strchr(specialinitial, c) != NULL);
}

