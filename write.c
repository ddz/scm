/* $Id$ */
/*
 * Scheme printer
 */

#include "types.h"

scheme_t scheme_write(scheme_t);

void write_list(scheme_t list)
{
    scheme_t car = SCHEME_CAR(list);
    scheme_t cdr = SCHEME_CDR(list);
    
    scheme_write(car);
    
    if (cdr != SCHEME_NIL) {
        printf(" ");
        write_list(cdr);
    }
}

scheme_t scheme_write(scheme_t obj)
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
                char* str = (char*)SCHEME_CDR(obj);
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
                printf("%s", SCHEME_CDR(obj));
                break;
            default:
                printf("#<heap pointer>");
            }
        }
    }

    return SCHEME_UNSPEC;
}
