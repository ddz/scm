/* $Id$ */
/*
 * Scheme symbol and RnRS symbol functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scheme.h"
#include "symtable.h"

#define INIT_TABLE_SIZE  101

static symtable_t* symbol_table = NULL;

/*
 * Make an interned symbol.  The stored symbol name is a copy of the
 * given string converted to all lowercase.
 */
scheme_t make_intern_symbol(char* str, size_t len)
{
    int i;
    char* name;
    scheme_t s = MAKE_CELL();
    scheme_set_carx(s, (len << 5) | (SYMBOL_T << 3) | 6);

    name = strdup(str);
    /* Symbols are stored all lowercase internally */
    for (i = 0; i < len; i++)
        name[i] = tolower(name[i]);
    
    scheme_set_cdrx(s, name);
    return s;
}

/*
 * Make a symbol with the given name.  If an interned symbol already
 * exists with the given name, it is returned instead.
 */
scheme_t make_symbol(char* name, size_t len)
{
    symtable_entry_t** e;

    if (symbol_table == NULL) {
        symbol_table = make_symtable(INIT_TABLE_SIZE);
    }
    
    e = symtable_lookup(symbol_table, name);

    if (*e == NULL) {
        *e = make_symtable_entry(make_intern_symbol(name, len));
        
        if (++symbol_table->used >
	    REHASH_THRESHOLD * symbol_table->size) {
            symtable_resize(symbol_table,
			    (symbol_table->size * 2) + 1);
	    e = symtable_lookup(symbol_table, name);
	}
    }
    
    return (*e)->symbol;
}
