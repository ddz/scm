/* $Id$ */
/*
 * Scheme symbol and RnRS symbol functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scheme.h"
#include "map.h"

static map_t* oblist = NULL;

/*
 * A case-insensitive hash_pjw
 */
size_t hash_symbol(const void* v)
{
    const char* p = (const char*)v;
    register unsigned int h = 0, g, l, n = strlen(p);
    for (l = n; l > 0; --l) {
        h = (h<<4) + tolower(*p++);  // XXX: is tolower necessary?
        if ((g = h&0xf0000000)) {
            h = h ^ (g>>24);
            h = h^g;
        }
    }
    return h&0xfffffff;
}

/*
 * Make a symbol with the given name.  If an interned symbol already
 * exists with the given name, it is returned instead.
 */
scheme_t make_symbol(char* name, size_t len)
{
    scheme_t s;
    
    if (oblist == NULL) {
        oblist = malloc(sizeof(map_t));
        map_init(oblist, hash_symbol, (map_cmp_t)strcmp);
    }

    if ((s = (scheme_t)map_get(oblist, name)) == NULL) {
        int i;
        char* intern_name = malloc(len + 1);

        for (i = 0; i < len; i++)
            intern_name[i] = tolower(name[i]);
        intern_name[i] = '\0';

        s = MAKE_CELL();
        scheme_set_carx(s, (len << 6) | (SYMBOL_T << 3) | 6);
        scheme_set_cdrx(s, intern_name);

        map_put(oblist, intern_name, (void*)s);
    }
    
    return s;
}

