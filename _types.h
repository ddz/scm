/* $Id$ */
/*
 * Private scheme data type representations
 */

#ifndef _TYPES_H
#define _TYPES_H

#include <sys/types.h>

typedef u_int32_t scheme_t;

/* Tag 0:
 *   000 - Immediate value (type determined by further stages)
 *   010 - Fixnum (immediate integer)
 *   100 - Heap cell pointer (type determined by further stages)
 *   110 - Heap pair pointer
 */

#define TAG_MASK ((1 << 2) | (1 << 1) | (1 << 0))
#define IMMVAL_T 0
#define FIXNUM_T ((IMMVAL_T) | (1 << 1))
#define CELLPTR_T (1 << 2)
#define PAIRPTR_T (CELL_PTR | (1 << 1))

#define GET_TAG(s) (s & TAG_MASK)
#define GET_FIXNUM(s) (s >> 3)
#define GET_PTR(s) ((scheme_t*)(s & ~TAG_MASK))

#define IS_IMMVAL(s) (GET_TAG(s) == IMMVAL_T)
#define IS_FIXNUM(s) (GET_TAG(s) == FIXNUM_T)
#define IS_CELL(s)   (GET_TAG(s) == CELLPTR_T)
#define IS_PAIR(s)   (GET_TAG(s) == PAIRPTR_T)

#define MAKE_FIXNUM(i) ((i << 3) | FIXNUM_T)
#define MAKE_CELLPTR(p) (p | CELLPTR_T)
#define MAKE_PAIRPTR(p) (p | PAIRPTR_T)

/*
 * Tag 1 (immediate value type tag):
 *   00 - boolean
 *   01 - character
 *   10 - unique
 *   11 - special form
 */

#define IMM_TAG_MASK ((1 << 4) | (1 << 3))
#define BOOL_T (0 << 3)
#define CHAR_T (1 << 3)
#define UNIQ_T (1 << 4)
#define SPEC_T ((1 << 4) | (1 << 3))

#define GET_IMM_TAG(s) (s & IMM_TAG_MASK)
#define GET_BOOL(s) (s & (1 << 5))
#define GET_CHAR(s) (s >> 16)
#define GET_UNIQ(s) (s >> 16)
#define GET_SPEC(s) (s >> 16)

#define IS_BOOL(s) (GET_IMM_TAG(s) == BOOL_T)
#define IS_CHAR(s) (GET_IMM_TAG(s) == CHAR_T)
#define IS_UNIQ(s) (GET_IMM_TAG(s) == UNIQ_T)
#define IS_SPEC(s) (GET_IMM_TAG(s) == SPEC_T)

#define MAKE_BOOL(b) ((b << 5)  | BOOL_T | IMMVAL_T)
#define MAKE_CHAR(c) ((c << 16) | CHAR_T | IMMVAL_T)
#define MAKE_UNIQ(u) ((u << 6)  | UNIQ_T | IMMVAL_T)
#define MAKE_SPEC(s) ((s << 16) | SPEC_T | IMMVAL_T)

#define BOOL_FALSE  MAKE_BOOL(0)
#define BOOL_TRUE   MAKE_BOOL(1)
#define UNIQ_NIL    MAKE_UNIQ(0)
#define UNIQ_EOF    MAKE_UNIQ(1)
#define UNIQ_UNSPEC MAKE_UNIQ(2)
#define UNIQ_UNDEF  MAKE_UNIQ(3)

typedef scheme_t* cell_t;

#define CLOSURE_T (1 << 0)
#define NUMBER_T  ((1 << 1) | (1 << 0))
#define BIGNUM_T  ((0 << 3) | NUMBER_T)
#define RATNUM_T  ((1 << 3) | NUMBER_T)
#define FLONUM_T  ((2 << 3) | NUMBER_T)
#define REALNUM_T ((3 << 3) | NUMBER_T)
#define RECTNUM_T ((4 << 3) | NUMBER_T)
#define COMPNUM_T ((5 << 3) | NUMBER_T)
#define LENPTR_T  ((1 << 2) | (1 << 0))
#define SYMBOL_T  ((0 << 3) | LENPTR_T)
#define STRING_T  ((1 << 3) | LENPTR_T)
#define VECTOR_T  ((2 << 3) | LENPTR_T)
#define PORT_T    ((3 << 3) | LENPTR_T)
#define OBJECT_T  ((1 << 2) | (1 << 1) | (1 << 0))

#define GET_CELL_TAG(s)    ((*GET_PTR(s)) & TAG_MASK)
#define GET_CELL_CAR(s)    ((*GET_PTR(s)) & ~1)
#define GET_CELL_CDR(s)    (*(GET_PTR(s) + 1))
#define GET_CELL_NUMBER(s) GET_CELL_CDR(s)
#define GET_CELL_LENGTH(s) (GET_CELL_CAR(s) >> 5)
#define GET_CELL_PTR(s)    GET_CELL_CDR(s)
#define GET_OBJECT_TYPE(s) (GET_CELL_CAR(s) >> 3)
#define GET_OBJECT_PTR(s)  GET_CELL_CDR(s)

#define IS_CLOSURE(s)  (GET_CELL_TAG(s) == CLOSURE_T)
#define IS_NUMBER(s)   (GET_CELL_TAG(s) == NUMBER_T)
#define IS_BIGNUM(s)   (GET_CELL_TAG(s) == BIGNUM_T)
#define IS_RATNUM(s)   (GET_CELL_TAG(s) == RATNUM_T)
#define IS_FLONUM(s)   (GET_CELL_TAG(s) == FLONUM_T)
#define IS_REALNUM(s)  (GET_CELL_TAG(s) == REALNUM_T)
#define IS_RECTNUM(s)  (GET_CELL_TAG(s) == RECTNUM_T)
#define IS_COMPNUM(s)  (GET_CELL_TAG(s) == COMPNUM_T)
#define IS_LENPTR(s)   (GET_CELL_TAG(s) == LENPTR_T)
#define IS_SYMBOL(s)   (GET_CELL_TAG(s) == SYMBOL_T)
#define IS_STRING(s)   (GET_CELL_TAG(s) == STRING_T)
#define IS_VECTOR(s)   (GET_CELL_TAG(s) == VECTOR_T)
#define IS_PORT(s)     (GET_CELL_TAG(s) == PORT_T)
#define IS_OBJECT(s)   (GET_CELL_TAG(s) == OBJECT_T)

#define SET_CELL_CAR(s, a)    (*GET_PTR(s) = a)
#define SET_CELL_CDR(s, d)    (*(GET_PTR(s) + 1) = d)
#define SET_CELL_TAG(s, t)    (*GET_PTR(s) |= t)
#define SET_CELL_LENGTH(s, n) (*GET_PTR(s) |= (n << 5))
#define SET_CELL_PTR(s, p)    (*(GET_PTR(s) + 1) = p)
#define SET_OBJECT_TYPE(s, t) (*GET_PTR(s) |= (t << 3))
#define SET_OBJECT_PTR(s, p)  SET_CELL_PTR(s, p)

#define MARK_CELL(s)      (*(GET_PTR(s) + 1) |= 1)
#define UNMARK_CELL(s)    (*(GET_PTR(s) + 1) &= ~1)
#define IS_CELL_MARKED(s) (*(GET_PTR(s) + 1) & 1)


#endif
