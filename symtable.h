/* $Id$ */
/*
 * Scheme symbol table.
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#define REHASH_THRESHOLD 0.75

/*
 * Symbol table entry containing symbol name and pointer to next
 * entry.
 */
typedef struct _entry {
    scheme_t       symbol;    /* The interned symbol */
    struct _entry* next; 
} symtable_entry_t;

/*
 * The symbol table is a simple separate chaining hash table.
 */
typedef struct {
    size_t             used;
    size_t             size;
    symtable_entry_t** table;
} symtable_t;

/*
 * Allocate and initialize a new symbol table of the given size.
 */
extern symtable_t* make_symtable(size_t);

/*
 * Resize and rehash the symbol table's hash table.
 */
extern int symtable_resize(symtable_t*, size_t);

/*
 * Lookup a symbol by name in the table.
 */
extern symtable_entry_t** symtable_lookup(symtable_t*, char*);

/*
 * Allocate and initialize a new symbol table entry.
 */
extern symtable_entry_t* make_symtable_entry(scheme_t);

#endif
