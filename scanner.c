/* $Id$ */

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include "scheme.h"
#include "stk.h"
#include "strbuf.h"

jmp_buf top_level;
char* error_msg;
#define error(msg) do { error_msg = msg; longjmp(top_level, 1); } while (0);

void read_atmosphere(FILE* f)
{
    char c;
    while ((c = getc(f)) != EOF) {
        switch (c) {
        case ';':
            while ((c = getc(f)) != EOF) {
                if (c == '\n')
                    break;
            }
            break;
            
        case ' ':
        case '\n':
            break;
            
        default:
            ungetc(c, f);
            return;
        }
    }
}

scheme_t read_list(FILE* f)
{
    stk_t    stk    = STK_INITIALIZER;
    scheme_t s, ls  = SCHEME_NIL;

    while ((s = read(f)) != SCHEME_NIL) {
        if (s == SCHEME_UNDEF) {
            ls = read(f);
            if ((s = read(f)) != SCHEME_NIL)
                error("Unexpected '.'");
            break;
        }
        else
            stk_push(&stk, (void*)s);
        
    }

    while (!stk_empty(&stk)) {
        scheme_t car = (scheme_t)stk_pop(&stk);
        ls = scheme_cons(car, ls);
    }
    
    return ls;
}

scheme_t read_vector(FILE* f)
{
    fprintf(stderr, "read_vector");
    return SCHEME_UNSPEC;
}

scheme_t read_character(FILE* f)
{
    fprintf(stderr, "read_character");
    return SCHEME_UNSPEC;
}

scheme_t read_number(FILE* f)
{
    return MAKE_FIXNUM(1);
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

scheme_t read(FILE* f)
{
    char c;

    read_atmosphere(f);
    
    c = getc(f);
    
    switch (c) {
    case EOF:
        return SCHEME_EOF;
        
    case '(':
        return read_list(f);
        
    case ')':
        return SCHEME_NIL;
        
    case '#': {
        char d = getc(f);
        switch (d) {
        case EOF:
            return SCHEME_EOF;
        case '(':
            return read_vector(f);
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
            
    case '\"':
        return read_string(f);

    case '.':
        return SCHEME_UNDEF;
        
    case ',':
    case '`':
    case '\'':
        return SCHEME_NIL;

    case '+':
    case '-':
        ungetc(c, f);
        return read_number(f);
        
    default:
        if (isdigit(c))
            return read_number(f);
        else
            printf("ERROR: Bad character %c\n", c);
    }
}

int main(int argc, char* argv[])
{
    scheme_t s;
    
    while (1) {
        printf("> ");
        if (setjmp(top_level) == 0) {
            if ((s = read(stdin)) == SCHEME_EOF)
                break;
            scheme_write_1(s);
        }
        else
            printf("ERROR: %s\n", error_msg);
        printf("\n");
    }
    return 0;
}
