/* $Id$ */

#include "scm_hash.h"

int scm_hash_init(scm_hash_t* h)
{
    h->magic = SCM_HASH_VALID;
    h->used  = 0;
    h->size  = 0;
    h->rehash_size =      2.0;
    h->rehash_threshold = 0.5;
    h->table = NULL;
    
    return 0;
}

int scm_hash_resize(scm_hash_t* h, size_t size)
{
    h->size = size;
    h->table = realloc(h->table, sizeof(scheme_t) * h->size);
    if (h->table == NULL) {
	printf("ERROR: Could not allocate memory\n");
	abort();
    }
}
