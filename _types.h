/* $Id$ */
/*
 * Private scheme data type representations
 */

#ifndef _TYPES_H
#define _TYPES_H

#include <sys/types.h>

/*
 * Scheme values.
 */

typedef u_int32_t scheme_t;

/*
 * Type tag: two-bit immediate/pointer type tag
 *   00 - Immediate integer (fixnum)
 *   01 - Immediate floating point (short float)
 *   10 - Other immediate value (char, bool, etc.)
 *   11 - Heap pointer
 */

#define TAG_MASK ((1 << 1) | (1 << 0))
#define FIXNUM_T  0
#define SHRTFL_T  1
#define IMMVAL_T  2
#define HEAPPTR_T 3

#define GET_TAG(s)   (s & TAG_MASK)
#define IS_FIXNUM(s)  (GET_TAG(s) == FIXNUM_T)
#define IS_SHRTFL(s)  (GET_TAG(s) == SHRTFL_T)
#define IS_IMMVAL(s)  (GET_TAG(s) == IMMVAL_T)
#define IS_HEAPPTR(s) (GET_TAG(s) == HEAPPTR_T)

#define GET_FIXNUM(i) (i >> 2)

#define MAKE_FIXNUM(i)  ((i << 2) | FIXNUM_T)
/* #define MAKE_SHRTFL(f) (...) */

/*
 * Other immediate values are distinguished by a tag in bits 3 and 4:
 *   00 - boolean
 *   01 - character
 *   10 - unique value
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

/*
 * Heap pointers:  Third bit distinguishes pair pointers from other
 *                 heap cells.
 */
#define PTRTAG_MASK ((1 << 2) | (1 << 1) | (1 << 0))
#define CELLPTR_T   ((0 << 2) | (1 << 1) | (1 << 0))
#define PAIRPTR_T   ((1 << 2) | (1 << 1) | (1 << 0))

#define GET_PTRTAG(s) (s & PTRTAG_MASK)
#define IS_CELLPTR(s) (GET_PTRTAG(s) == CELLPTR_T)
#define IS_PAIRPTR(s) (GET_PTRTAG(s) == PAIRPTR_T)

#define GET_PTR(s) (s & ~(PTRTAG_MASK))

#define MAKE_CELLPTR(p) (p | CELLPTR_T)
#define MAKE_PAIRPTR(p) (p | PAIRPTR_T)

/*
 * Scheme Heap cells
 */

typedef scheme_t cell_t[2];

/*
 * The cell type is assumed to be a pair unless the low three bits of
 * the first cell word are 110.
 */

#define GET_CELLTAG(s) ((SCHEME_CAR(s) >> 3) & 3)
#define GET_CELLLEN(s) ((SCHEME_CAR(s) >> 5))

#define SYMBOL_T 0
#define STRING_T 1

#define GET_CAR(c) (((scheme_t*)c)[0])
#define GET_CDR(c) (((scheme_t*)c)[1])

#define MAKE_CELL() (MAKE_CELLPTR(malloc(sizeof(cell_t))))
#define MAKE_PAIR() (MAKE_PAIRPTR(malloc(sizeof(cell_t))))

#endif
