/* $Id$ */

#include <stdlib.h>
#include "scheme.h"
#include "symtable.h"

symtable_t* make_symtable(size_t size)
{
    symtable_t* st = malloc(sizeof(symtable_t));
    symtable_init(st, size);
    return st;
}

int symtable_init(symtable_t* st, size_t size)
{
    st->used = 0;
    st->size = size;
    st->table = malloc(st->size * sizeof(symtable_entry_t*));
}

int symtable_resize(symtable_t* st, size_t size)
{
    int i, old_size = st->size;
    symtable_entry_t** old_table = st->table;

    st->size  = size;
    if (!(st->table = malloc(st->size * sizeof(symtable_entry_t*)))) {
	printf("symtable_resize: unable to allocate memory\n");
        return -1;
    }

    for (i = 0; i < old_size; i++) {
        if (old_table[i]) {
            symtable_entry_t* e = old_table[i];

            while (e) {
                symtable_entry_t* f = e->next;
                symtable_entry_t** pe =
                    symtable_lookup(st, GET_SYMBOL_NAME(e->symbol));
                *pe = e;
                e->next = NULL;
                e = f;
            }
        }
    }

    free(old_table);

    return 0;
}

/*
 * hash_pjw
 */
size_t hash_string(char* p)
{
    register unsigned int h = 0, g, l, n = strlen(p);
    for (l = n; l > 0; --l) {
        h = (h<<4) + *p++;
        if ((g = h&0xf0000000)) {
            h = h ^ (g>>24);
            h = h^g;
        }
    }
    return h&0xfffffff;

}

symtable_entry_t** symtable_lookup(symtable_t* st, char* name)
{
    int i;
    symtable_entry_t** e;

    i = hash_string(name) % st->size;
    
    for (e = &st->table[i]; *e; e = &(*e)->next) {
        char* s = GET_SYMBOL_NAME((*e)->symbol);
        if (strcasecmp(s, name) == 0)
            return e;
    }

    return e;
}

symtable_entry_t* make_symtable_entry(scheme_t symbol)
{
    symtable_entry_t* e;
    e = malloc(sizeof(symtable_entry_t));
    e->symbol = symbol;
    e->next = NULL;
        
    return e;
}

int symtable_insert(symtable_t* st, scheme_t symbol)
{
    symtable_entry_t** e = symtable_lookup(st, GET_SYMBOL_NAME(symbol));

    if (*e)
        return -1;
    else {
        *e = make_symtable_entry(symbol);
        
        if (++st->used > REHASH_THRESHOLD * st->size)
            symtable_resize(st, (st->size * 2) + 1);
    
        return 0;
    }
}

scheme_t symtable_get(symtable_t* st, char* name)
{
    symtable_entry_t** e = symtable_lookup(st, name);

    if (*e)
        return (*e)->symbol;
    return SCHEME_UNDEF;
}
