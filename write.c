/* $Id$ */
/*
 * Scheme printer
 */

#include <stdio.h>
#include "scheme.h"

char* synt_names[] = {
    "quote", "lambda", "if", "set!", "begin", "cond", "and", "or",
    "case", "let", "let*", "letrec", "do", "delay", "quasiquote",
    "else", "=>", "define", "unquote", "unquote-splicing"
};

void write_list(scheme_t list)
{
    scheme_t car = scheme_car(list);
    scheme_t cdr = scheme_cdr(list);

    if (car != SCHEME_NIL)
	scheme_write_1(car);

    if (cdr != SCHEME_NIL) {
        printf(" ");
	if (scheme_pairp(cdr))
	    write_list(cdr);
	else {
	    printf(". ");
	    scheme_write_1(cdr);
	}
    }
}

void write_vector(scheme_t vec)
{
    int i;
    size_t elems = GET_CELLLEN(vec);
    scheme_t* vector = (scheme_t*)scheme_cdr(vec);

    for (i = 0; i < elems; i++) {
	scheme_write_1(vector[i]);
        if (i != elems - 1)
            printf(" ");
    }
}

scheme_t scheme_write_1(scheme_t obj)
{
    switch (GET_TAG(obj)) {
    case FIXNUM_T:
        printf("%d", GET_FIXNUM(obj));
        break;
    case SHRTFL_T:
        break;
    case IMMVAL_T: {
        switch (GET_IMM_TAG(obj)) {
        case BOOL_T:
            if (obj == SCHEME_TRUE)
                printf("#t");
            else
                printf("#f");
            break;
            
        case CHAR_T: {
            char c = GET_CHAR(obj);
            switch (c) {
            case ' ':
                printf("#\\space");
                break;
            case '\n':
                printf("#\\newline");
                break;
            default:
                printf("#\\%c", c);
                break;
            }
            break;
        }
            
        case UNIQ_T:
            if (obj == SCHEME_NIL)
                printf("()");
            else if (obj == SCHEME_EOF)
                printf("#<eof>");
            else if (obj == SCHEME_UNSPEC)
                break;
            else
                printf("#<undefined>");
            break;
            
        case SYNT_T:
	    printf("%s", synt_names[GET_SYNT(obj)]);
            break;
        }
        break;
    }
        
    case HEAPPTR_T:
        if (GET_PTRTAG(obj) == PAIRPTR_T) {
            printf("(");
            write_list(obj);
            printf(")");
        }
        else {
            switch (GET_CELLTAG(obj)) {
            case STRING_T: {
                int i, len = GET_CELLLEN(obj);
                char* str = (char*)scheme_cdr(obj);
                printf("\"");
                for (i = 0; i < len; i++) {
                    switch (str[i]) {
                    case '\\':
                        printf("\\\\");
                        break;
                    case '"':
                        printf("\\\"");
                        break;
                    default:
                        printf("%c", str[i]);
                    }
                }
                printf("\"");
                break;
            }
                
            case SYMBOL_T:
                printf("%s", (char*)scheme_cdr(obj));
                break;

	    case VECTOR_T:
		printf("#(");
		write_vector(obj);
		printf(")");
		break;
		
            default:
                printf("#<heap pointer>");
            }
        }
        break;
    }

    return SCHEME_UNSPEC;
}
