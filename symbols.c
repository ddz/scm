/* $Id$ */
/*
 * Scheme symbol and RnRS symbol functions
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scheme.h"

#define REHASH_THRESHOLD 0.5

typedef struct _entry {
    scheme_t       symbol;    /* The interned symbol */
    struct _entry* next; 
} symtable_entry_t;

typedef struct {
    size_t             used;
    size_t             size;
    symtable_entry_t** table;
} symtable_t;

static symtable_t* symbol_table = NULL;

static symtable_t* make_symtable(size_t);
static symtable_init(symtable_t*, size_t);
static int symtable_rehash(symtable_t*);
static size_t hash_string(char*);
static symtable_entry_t** symtable_lookup(symtable_t*, char*);
static symtable_entry_t* make_symtable_entry(scheme_t);
static int symtable_insert(symtable_t*, scheme_t);
static scheme_t symtable_get(symtable_t*, char*);
static scheme_t make_intern_symbol(char*);


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

int symtable_rehash(symtable_t* st)
{
    int i, old_size = st->size;
    symtable_entry_t** old_table = st->table;

    st->size  = (old_size * 2) + 1;
    if (!(st->table = malloc(st->size * sizeof(symtable_entry_t*)))) {
        /* XXX: Unable to allocate memory */
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
            symtable_rehash(st);
    
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

scheme_t make_intern_symbol(char* name)
{
    int i, len = strlen(name);
    scheme_t s = MAKE_CELL();
    scheme_set_carx(s, (len << 5) | (SYMBOL_T << 3) | 6);

    /* XXX: Symbols are stored all lowercase internally */
    for (i = 0; i < len; i++)
        name[i] = tolower(name[i]);
    
    scheme_set_cdrx(s, name);
    return s;
}

scheme_t make_symbol(char* name, size_t len)
{
    symtable_entry_t** e;

    if (symbol_table == NULL)
        symbol_table = make_symtable(101);
    
    e = symtable_lookup(symbol_table, name);

    if (*e == NULL) {
        *e = make_symtable_entry(make_intern_symbol(name));
        
        if (++symbol_table->used > REHASH_THRESHOLD * symbol_table->size)
            symtable_rehash(symbol_table);
    }
    
    return (*e)->symbol;
}
