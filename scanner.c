/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include "scheme.h"
#include "stk.h"
#include "strbuf.h"

scheme_t read_character(FILE* f)
{
    char c;
    
    fprintf(stderr, "read_character");
    
    while ((c = getc(f)) != EOF) {
	switch (c) {
	case ' ':
	case '\n':
	    break;
	    
	case '(':
	case ')':
	case '\"':
	case ';':
	    ungetc(c, f);
	    break;
	}
    }
	
    return SCHEME_UNSPEC;
}

scheme_t read_number(FILE* f)
{
    char c;
    static strbuf_t sb = STRBUF_INITIALIZER;

    strbuf_reset(&sb);
    
    while ((c = getc(f)) != EOF) {
	switch (c) {
	case '(':
	case ')':
	case '\"':
	case ';':
	    ungetc(c, f);

	case ' ':
	case '\n':
	    return MAKE_FIXNUM(atoi(strbuf_buffer(&sb)));

	default:
	    strbuf_add(&sb, c);
	}
    }
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

scheme_t scheme_read(FILE* f)
{
    char c;
    scheme_t s;
    
    typedef struct {
	enum {LIST, VECTOR} type;
	union {
	    struct {
		scheme_t  head;
		scheme_t  tail;
	    } list;
	    struct {
		size_t    used;
		size_t    size;
		scheme_t* v;
	    } vector;
	} seq;
    } sequence_state_t;

    sequence_state_t* seq;
    stk_t stk = STK_INITIALIZER;
    
    while ((c = getc(f)) != EOF) {
	s = SCHEME_UNDEF;
	
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

	case '(':
	    seq = malloc(sizeof(sequence_state_t));
	    seq->type = LIST;
	    seq->seq.list.head = seq->seq.list.tail = SCHEME_NIL;
	    stk_push(&stk, seq);
	    continue;
	    
	case '.': {
	    char d, e;
	    if ((c = getc(f)) != '.') {
		if (stk_empty(&stk))
		    error("Unexpected '.'");
		seq = stk_top(&stk);
		if (seq->type != LIST)
		    error("Unexpected '.'");
		s = scheme_read(f);
		if (s == SCHEME_EOF)
		    return s;
		scheme_set_cdrx(seq->seq.list.tail, s);
		continue;
	    }
	    else if ((d = getc(f)) == '.') {
		s = MAKE_SYMBOL("...", 3);
		break;
	    }
	    else
		error("Unexpected '.'");
	}
	
	case ')':
	    seq = (sequence_state_t*)stk_pop(&stk);
	    if (seq->type == LIST)
		s = seq->seq.list.head;
	    else {
		seq->seq.vector.v = realloc(seq->seq.vector.v,
					 seq->seq.vector.used);
		s = MAKE_VECTOR(seq->seq.vector.v, seq->seq.vector.used);
	    }
	    break;

	case '\"':
	    s = read_string(f);
	    break;

	/*
	 * Abbreviations
	 */
	case ',': {
	    char d = getc(f);
	    if (d == '@') {
		s = scheme_read(f);
		s = scheme_cons(SCHEME_UNQUOTE_SPLICING,
				scheme_cons(s, SCHEME_NIL));
	    }
	    else {
		ungetc(d, f);
		s = scheme_read(f);
		s = scheme_cons(SCHEME_UNQUOTE,
				scheme_cons(s, SCHEME_NIL));
	    }
	    break;
	}
	    
	case '`':
	    s = scheme_read(f);
	    if (s != SCHEME_EOF)
		s = scheme_cons(SCHEME_QUASIQUOTE,
				scheme_cons(s, SCHEME_NIL));
	    break;
	    
	case '\'':
	    s = scheme_read(f);
	    if (s != SCHEME_EOF)
		s = scheme_cons(SCHEME_QUOTE,
				scheme_cons(s, SCHEME_NIL));
	    break;
	    
	case '+':
	case '-': {
	    char d = getc(f);
	    if (isdigit(d)) {
		ungetc(d, f);
		ungetc(c, f);
		s = read_number(f);
		break;
	    }
	    ungetc(c, f);
	}
	    
	case '#': {
	    char d = getc(f);
	    switch (d) {
	    case EOF:
		return SCHEME_EOF;
		
	    case '(':
		seq = malloc(sizeof(sequence_state_t));
		seq->type = VECTOR;
		seq->seq.vector.used = 0;
		seq->seq.vector.size = 0;
		seq->seq.vector.v = NULL;
		stk_push(&stk, seq);
		break;
		
	    case 't':
	    case 'T':
		s = SCHEME_TRUE;
		break;
		
	    case 'f':
	    case 'F':
		s = SCHEME_FALSE;
		break;
		
	    case '\\':
		s = read_character(f);
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
		s = read_number(f);
		break;
		
	    default:
		ungetc(d, f);
		ungetc(c, f);
		s =  read_number(f);
		break;
	    }
	    break;
	}
        
	default:
	    if (isdigit(c)) {
		ungetc(c, f);
		s = read_number(f);
	    }
	    else if (isinitial(c)) {
		ungetc(c, f);
		s = read_identifier(f);
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

		seq->seq.vector.v[seq->seq.vector.size++] = s;
	    }
	}
	else
	    return s;
    }

    return SCHEME_EOF;
}
