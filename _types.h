/* $Id$ */
/*
 * Private scheme data type representations
 */

#ifndef _TYPES_H
#define _TYPES_H

#include <sys/types.h>

/*
 * The data representation used is a hybrid using tagged words and
 * double wrappers.  Tag bits in the low two bits of the word indicate
 * whether it is a pointer to a heap cell or an immediate value (and
 * what type of immediate value it is).  In the first word of a heap
 * cell, a specific tag bit value indicates that further type
 * information is contained in a second-stage tag.
 *
 * The extended heap cell tag indicates that an additional two-bit
 * type type tag is in bits 3 and 4.  For types represented by a
 * length and a pointer (symbols, strings, and vectors), the length is
 * contained in the remaining bits of the word with the pointer to the
 * first element in the second word of the heap cell.  Other objects
 * may contain extra information in the first word with their pointer
 * in the second word.
 *
 * A type distinguisher may be built by collapsing the various staged
 * type tags in the following manner:
 *
 *  O...OHHIIITT
 *  |   ||||||\\-- Immediate/pointer type tag
 *  |   |||\\\---- Immediate value type tag
 *  |   |\\------- Heap cell type tag
 *  \...\--------- Object type tag
 *
 * This distinguisher is rather sparse, so values for it should be
 * checked exactly.  This may be changed to a denser distinguisher
 * later.
 */

/*
 * Scheme values.
 */

typedef int32_t scheme_t;

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
 *   11 - syntactic keyword
 */
#define IMM_TAG_MASK ((1 << 4) | (1 << 3))
#define BOOL_T (0 << 3)
#define CHAR_T (1 << 3)
#define UNIQ_T (1 << 4)
#define SYNT_T ((1 << 4) | (1 << 3))

#define GET_IMM_TAG(s) (s & IMM_TAG_MASK)
#define GET_BOOL(s) (s & (1 << 5))
#define GET_CHAR(s) (s >> 16)
#define GET_UNIQ(s) (s >> 16)
#define GET_SYNT(s) (s >> 16)

#define IS_BOOL(s) (GET_IMM_TAG(s) == BOOL_T)
#define IS_CHAR(s) (GET_IMM_TAG(s) == CHAR_T)
#define IS_UNIQ(s) (GET_IMM_TAG(s) == UNIQ_T)
#define IS_SYNT(s) (GET_IMM_TAG(s) == SYNT_T)

#define MAKE_BOOL(b) ((b << 5)  | BOOL_T | IMMVAL_T)
#define MAKE_CHAR(c) ((c << 16) | CHAR_T | IMMVAL_T)
#define MAKE_UNIQ(u) ((u << 6)  | UNIQ_T | IMMVAL_T)
#define MAKE_SYNT(s) ((s << 16) | SYNT_T | IMMVAL_T)

#define BOOL_FALSE            MAKE_BOOL(0)
#define BOOL_TRUE             MAKE_BOOL(1)
#define UNIQ_NIL              MAKE_UNIQ(0)
#define UNIQ_EOF              MAKE_UNIQ(1)
#define UNIQ_UNSPEC           MAKE_UNIQ(2)
#define UNIQ_UNDEF            MAKE_UNIQ(3)
#define SYNT_QUOTE            MAKE_SYNT(0)
#define SYNT_LAMBDA           MAKE_SYNT(1)
#define SYNT_IF               MAKE_SYNT(2)
#define SYNT_SETX             MAKE_SYNT(3)
#define SYNT_BEGIN            MAKE_SYNT(4)
#define SYNT_COND             MAKE_SYNT(5)
#define SYNT_AND              MAKE_SYNT(6)
#define SYNT_OR               MAKE_SYNT(7)
#define SYNT_CASE             MAKE_SYNT(8)
#define SYNT_LET              MAKE_SYNT(9)
#define SYNT_LETS             MAKE_SYNT(10)
#define SYNT_LETREC           MAKE_SYNT(11)
#define SYNT_DO               MAKE_SYNT(12)
#define SYNT_DELAY            MAKE_SYNT(13)
#define SYNT_QUASIQUOTE       MAKE_SYNT(14)
#define SYNT_ELSE             MAKE_SYNT(15)
#define SYNT_EQGT             MAKE_SYNT(16)
#define SYNT_DEFINE           MAKE_SYNT(17)
#define SYNT_UNQUOTE          MAKE_SYNT(18)
#define SYNT_UNQUOTE_SPLICING MAKE_SYNT(19)

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

#define MAKE_CELLPTR(p) ((unsigned int)p | CELLPTR_T)
#define MAKE_PAIRPTR(p) ((unsigned int)p | PAIRPTR_T)

/*
 * Scheme Heap cells
 */

typedef scheme_t cell_t[2];

/*
 * The cell type is assumed to be a pair unless the low three bits of
 * the first cell word are 110 (which indicates further type
 * information is contained in a second-stage tag).
 *
 * This section may change in the future when heap storage is
 * implemented.
 */

#define GET_CELLTAG(s) ((GET_CAR(GET_PTR(s)) >> 3) & 3)
#define GET_CELLLEN(s) ((GET_CAR(GET_PTR(s)) >> 5))

#define SYMBOL_T 0
#define STRING_T 1
#define VECTOR_T 2

#define GET_CAR(c)         (((scheme_t*)c)[0])
#define GET_CDR(c)         (((scheme_t*)c)[1])
#define GET_SYMBOL_NAME(s) ((char*)GET_CDR(GET_PTR(s)))
#define IS_SYMBOL(s)       (IS_CELLPTR(s) && GET_CELLTAG(s) == SYMBOL_T)

#define MAKE_CELL() (MAKE_CELLPTR(malloc(sizeof(cell_t))))
#define MAKE_PAIR() (MAKE_PAIRPTR(malloc(sizeof(cell_t))))
#define MAKE_SYMBOL(str, size) (make_symbol(str, size))
#define MAKE_STRING(str, size) (make_string(str, size))
#define MAKE_VECTOR(vec, elms) (make_vector(vec, elms))

extern scheme_t make_symbol(char*, size_t);
extern scheme_t make_string(char*, size_t);
extern scheme_t make_vector(scheme_t*, size_t);

#endif
