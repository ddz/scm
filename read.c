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

scheme_t read_tmp = SCHEME_NIL;
sequence_state_t* seq = NULL;
stk_t stk = STK_INITIALIZER;

scheme_t scheme_read(FILE* f)
{
    char c;

    while ((c = getc(f)) != EOF) {
	read_tmp = SCHEME_UNDEF;

	switch (c) {
	/*
	 * Atmosphere (skip whitespace and comments)
	 */
	case ' ':
	case '\n':
	    continue;
	case ';':
	    do { c = getc(f); } while (c != '\n' && c != EOF);
	    continue;

	/*
	 * Begin reading a new list
	 */
	case '(':
	    seq = malloc(sizeof(sequence_state_t));
	    seq->type = LIST;
	    seq->seq.list.head = seq->seq.list.tail = SCHEME_NIL;
	    stk_push(&stk, seq);
	    continue;

	/*
	 * A '.' may be part of a dotted pair or the peculiar
	 * identifier '...'.
	 */
	case '.': {
	    char d, e;
	    if ((c = getc(f)) != '.') {
		if (stk_empty(&stk) ||
		    (seq = stk_top(&stk))->type != LIST)
		    error("Unexpected '.'");

		read_tmp = scheme_read(f);
		if (read_tmp == SCHEME_EOF)
		    return read_tmp;
		scheme_set_cdrx(seq->seq.list.tail, read_tmp);
		continue;
	    }
	    else if ((d = getc(f)) == '.') {
		read_tmp = MAKE_SYMBOL("...", 3);
		break;
	    }
	    else
		error("Unexpected '.'");
	}

	/*
	 * A sequence (list or vector) closing.
	 */
	case ')':
            if (stk_empty(&stk))
                error("Unbalanced parenthesis");
	    seq = (sequence_state_t*)stk_pop(&stk);
	    if (seq->type == LIST)
		read_tmp = seq->seq.list.head;
	    else {
		seq->seq.vector.v =
                    realloc(seq->seq.vector.v,
                            seq->seq.vector.used * sizeof(scheme_t));
		read_tmp = MAKE_VECTOR(seq->seq.vector.v, seq->seq.vector.used);
	    }
	    break;

	case '\"':
	    read_tmp = read_string(f);
	    break;

	/*
	 * Abbreviations
	 */
	case ',': {
	    char d = getc(f);
	    if (d == '@') {
		read_tmp = scheme_read(f);
		read_tmp = scheme_cons(MAKE_SYMBOL("unquote-splicing", 16),
				scheme_cons(read_tmp, SCHEME_NIL));
	    }
	    else {
		ungetc(d, f);
		read_tmp = scheme_read(f);
		read_tmp = scheme_cons(MAKE_SYMBOL("unquote", 7),
				scheme_cons(read_tmp, SCHEME_NIL));
	    }
	    break;
	}
	    
	case '`':
	    read_tmp = scheme_read(f);
	    if (read_tmp != SCHEME_EOF)
		read_tmp = scheme_cons(MAKE_SYMBOL("quasiquote", 10),
				scheme_cons(read_tmp, SCHEME_NIL));
	    break;
	    
	case '\'':
	    read_tmp = scheme_read(f);
	    if (read_tmp != SCHEME_EOF)
		read_tmp = scheme_cons(MAKE_SYMBOL("quote", 5),
				scheme_cons(read_tmp, SCHEME_NIL));
	    break;
	    
	case '+':
	case '-': {
	    char d = getc(f);
	    ungetc(d, f);

	    if (isdelimiter(d)) {
		if (c == '+')
		    read_tmp = MAKE_SYMBOL("+", 1);
		else
		    read_tmp = MAKE_SYMBOL("-", 1);
	    }
	    else {
		ungetc(c, f);
		read_tmp = read_number(f);
	    }
            break;
	}

	/*
	 * Lisp-like read macros
	 */
	case '#': {
	    char d = getc(f);
	    switch (d) {
	    case EOF:
		return SCHEME_EOF;

	    /*
	     * Opening a vector sequence
	     */
	    case '(':
		seq = malloc(sizeof(sequence_state_t));
		seq->type = VECTOR;
		seq->seq.vector.used = 0;
		seq->seq.vector.size = 0;
		seq->seq.vector.v = NULL;
		stk_push(&stk, seq);
		continue;
		
	    case 't':
	    case 'T':
		read_tmp = SCHEME_TRUE;
		break;
		
	    case 'f':
	    case 'F':
		read_tmp = SCHEME_FALSE;
		break;
		
	    case '\\':
		read_tmp = read_character(f);
		break;

	    case 'B':
	    case 'b':
	    case 'D':
	    case 'd':
	    case 'H':
	    case 'h':
	    case 'O':
	    case 'o':
	    case 'e':
	    case 'i':
		ungetc(d, f);
		ungetc(c, f);
		read_tmp = read_number(f);
		break;
		
	    default:
		ungetc(d, f);
		ungetc(c, f);
		read_tmp = read_number(f);
		break;
	    }
	    break;
	}
        
	default:
	    if (isdigit(c)) {
		ungetc(c, f);
		read_tmp = read_number(f);
	    }
	    else if (isinitial(c)) {
		ungetc(c, f);
		read_tmp = read_identifier(f);
	    }
	    else
		error("Bad character");
	}

	/*
	 * If s has been read (not still SCHEME_UNDEF), add it to the
	 * current sequence.  If there is no current sequence,
	 * return s.
	 */

	if (!stk_empty(&stk)) {
	    seq = (sequence_state_t*)stk_top(&stk);
	    if (seq->type == LIST) {
		scheme_t c = scheme_cons(read_tmp, SCHEME_NIL);
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

		seq->seq.vector.v[seq->seq.vector.used++] = read_tmp;
	    }
	}
	else
	    return read_tmp;
    }

    return SCHEME_EOF;
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

    s =  MAKE_SYMBOL(strbuf_buffer(&sb), strbuf_length(&sb));
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

